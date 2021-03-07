
#include "http_home.h"
#include "util.h"
using namespace std;

HttpHome::HttpHome(int port)
:port_(port), db_(&DB::instance())
{
    // arm64 use arm version node+sqlite3
    server_ = make_shared<HttpServer>();
    server_->config.port = port;
    std::size_t t_cnt = std::max( 2, (int)std::thread::hardware_concurrency() - 1 );
    server_->config.thread_pool_size = t_cnt;
    init();
}
HttpHome::~HttpHome()
{
    server_->stop();
}

void HttpHome::init()
{    
    handle_sql();
    handle_cors();
    static_dir();
    check_pass();
    login();
    emplace_ws();
    server_->start();
}

void HttpHome::static_dir()
{
    server_->default_resource["GET"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            fs::path home_root = AP::instance().home_path();
            auto path = home_root / request->path;
            if (fs::is_directory(path))
            {
                path /= "index.html";
            }               
            else 
            if( !fs::exists(path) )
            {
                // This is for spa routing
                path = home_root / "index.html";
            }    
            if( !fs::exists(path) )
            {
                throw invalid_argument("index.html not exist");
            } 
            SimpleWeb::CaseInsensitiveMultimap header; 
            header.emplace("Cache-Control", "max-age=86400");           
            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
            if (*ifs)
            {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
                // determine type by extension
                header.emplace("Content-Type", Util::mime_type(path.string()) );
                response->write(header);
                // Trick to define a recursive function within this scope (for example purposes)
                class FileServer
                {
                  public:
                    static void read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs)
                    {
                        // Read and send 128 KB at a time
                        vector<char> buffer(131072);
                        streamsize read_length;
                        if ((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) > 0)
                        {
                            response->write(&buffer[0], read_length);
                            if (read_length == static_cast<streamsize>(buffer.size()))
                            {
                                response->send([response, ifs](const SimpleWeb::error_code &ec) {
                                    if (!ec)
                                        read_and_send(response, ifs);
                                    else
                                        LOGE("get home file connection interrupted");
                                });
                            }
                        }
                    }
                };
                FileServer::read_and_send(response, ifs);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e)
        {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
        }
    };
}

void HttpHome::handle_sql()
{
    server_->resource["^/sql$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
    {        
        try
        {
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json;charset=utf-8");
            header.emplace("Connection", "keep-alive");
            header.emplace("Access-Control-Allow-Origin", "*");
            // throw exception hurt performance, so try to not use: try...catch...throw
            auto data = Json::parse(request->content);
            const auto pass = data["pass"].get<string>();
            auto sql = data["sql"].get<string>();           
            // LOGI("handle_sql : pass=%s; sql=%s", pass.c_str(), sql.c_str());  
            std::size_t pos = sql.find(";");
            if(pos != string::npos)
            {
                sql = sql.substr(0, pos+1 );
            }  
            sql = std::regex_replace(sql, std::regex("\\s{2,}"), " ");
            // for wrk test log a lot
            // LOGI("handle_sql : pass=%s; sql=%s", pass.c_str(), sql.c_str());  	
            const auto [pass0, pass1] = db_->get_pass();
            // todo: substr first sql that before ;
            if( boost::ifind_first(sql, "select ") && pass1 == pass)
            {
                pos = sql.find("from user");
                if(pos != string::npos)
                {
                    throw invalid_argument("permission denied");
                }  
                response->write(db_->exec_sql(sql), header);
            }
            else
            if(pass0 == pass) 
            {
                response->write(db_->exec_sql(sql), header);
            }
            else 
            {
                Json res;
                res["ret"] = -1;
                res["msg"] = "permission denied";
                response->write(res.dump(), header);
            }
        }
        catch (const exception &e)
        {
            Json res;
            res["ret"] = -1;
            res["msg"] = e.what();
            res_json(response, res);
        }   
    };
}
void HttpHome::res_json(std::shared_ptr<HttpServer::Response> response, Json &data)
{
    SimpleWeb::CaseInsensitiveMultimap header;
    header.emplace("Content-Type", "application/json;charset=utf-8");
    header.emplace("Connection", "keep-alive");
    header.emplace("Access-Control-Allow-Origin", "*");
    response->write(data.dump(), header);
}
void HttpHome::handle_cors()
{
    // Deals with CORS requests
    server_->default_resource["OPTIONS"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try {
            // Set header fields
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "text/plain; charset=utf-8");
            header.emplace("Access-Control-Allow-Origin", "*");
            header.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE");
            header.emplace("Access-Control-Max-Age", "1728000");
            header.emplace("Access-Control-Allow-Headers", "Authorization, Origin, X-Requested-With, Content-Type, Accept");

            response->write(SimpleWeb::StatusCode::success_ok, "", header);
        }
        catch(const exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
        }
    };
}
void HttpHome::login()
{
    server_->resource["^/login$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        Json res;
        try
        {
            auto data = Json::parse(request->content);
            auto usr = data["usr"].get<string>();
            auto pass = data["pass"].get<string>();
            const auto [usr0, pass0] = db_->get_acc();
            if(usr == usr0 && pass == pass0) 
            {
                const auto [pass0, pass1] = db_->get_pass();
                res["ret"] = 0;
                res["admin"] = pass0;
            }
            else 
            {
                throw invalid_argument("invalid account");
            }
        }
        catch (const exception &e)
        {
            res["ret"] = -1;
            res["msg"] = e.what();
        }
        res_json(response, res);
    };
}
void HttpHome::check_pass()
{
    server_->resource["^/check_pass$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        Json res;
        try
        {
            auto data = Json::parse(request->content);
            auto pass = data["pass"].get<string>();
            const auto [pass0, pass1] = db_->get_pass();
            if(pass0 == pass) 
            {
                res["ret"] = 0;
            }
            else 
            {
                throw invalid_argument("invalid pass");
            }
        }
        catch (const exception &e)
        {
            res["ret"] = -1;
            res["msg"] = e.what();
        }
        res_json(response, res);
    };
}
void HttpHome::emplace_ws()
{
    auto &bc_ep = ws_server_.endpoint["^/broadcast/?$"];
    bc_ep.on_open = [&](shared_ptr<WsServer::Connection> connection) {

    };
    bc_ep.on_close = [&](shared_ptr<WsServer::Connection> connection, int status, const string &reason) {

    };
    bc_ep.on_error = [&](shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec) {

    };
    bc_ep.on_message = [&](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
        auto msg = in_message->string();
        if(msg == "") return;
        for (auto &a_connection : bc_ep.get_connections())
        {
            a_connection->send(msg); 
        }                    
    };
    server_->on_upgrade = [this](unique_ptr<SimpleWeb::HTTP> &socket, shared_ptr<HttpServer::Request> request) {
        auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
        connection->method = std::move(request->method);
        connection->path = std::move(request->path);
        connection->http_version = std::move(request->http_version);
        connection->header = std::move(request->header);
        ws_server_.upgrade(connection);
    };
}
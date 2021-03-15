
#include "http_home.h"
#include "util.h"
using namespace std;

HttpHome::HttpHome(int port)
:port_(port), db_(&DB::instance()), app_(std::make_shared<uWS::App>())
{
    // arm64 use arm version node+sqlite3
}

void HttpHome::run()
{    
    loop_ = uWS::Loop::get();
    app_->get("/*", [this](auto *res, auto *req) {static_dir(res, req);})
        .post("/sql", [this](auto *res, auto *req) {handle_sql(res, req);})
        .post("/login", [this](auto *res, auto *req) {login(res, req);})
        .post("/check_pass", [this](auto *res, auto *req) {check_pass(res, req);})
        .options("/*", [this](auto *res, auto *req) {handle_cors(res, req);})
        .ws<PerSocketData>("/broadcast",  {/* Settings */
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024 * 1024,
        .idleTimeout = 16,
        .maxBackpressure = 1 * 1024 * 1024,
        .closeOnBackpressureLimit = false,
        .resetIdleTimeoutOnSend = false,
        .sendPingsAutomatically = true,
        /* Handlers */
        .upgrade = nullptr,
        .open = [this](auto *ws) {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
            ws->subscribe("broadcast"); 
        },
        .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
            /* Simply broadcast every single message we get */
            app_->publish("broadcast", message, opCode, true);
        },
        .drain = [](auto * /*ws*/) {
            /* Check ws->getBufferedAmount() here */ 
        },
        .ping = [](auto * /*ws*/, std::string_view) {
            /* Not implemented yet */ 
        },
        .pong = [](auto * /*ws*/, std::string_view) {
            /* Not implemented yet */ 
        },
        .close = [this](auto * /*ws*/, int /*code*/, std::string_view /*message*/) {
            /* You may access ws->getUserData() here */
        }
        }).listen(port_, [this](auto *token) {
            if (token)
            {
                LOG("home spa server listen at %1%", port_)
            }
        }).run();
}

void HttpHome::static_dir(auto *res, auto *req)
{
    try
    {
        fs::path home_root = AP::instance().home_path();
        auto path = home_root / string(req->getUrl());
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
        make_shared<AsyncFileStreamer>(path.string(), res, req)->streamFile();
    }
    catch (const exception &e)
    {
        res->writeStatus("404 Not Found");
        res->writeHeader("Content-Type", "text/html; charset=utf-8");
        res->end( boost::str(boost::format("<h1>404 Not Found</h1><i>%1%</i>") % e.what() ) );
    }

}
void HttpHome::get_post_data(auto *res, std::function<void(std::string)> dealer)
{
    // https://unetworking.github.io/uWebSockets.js/generated/interfaces/httprequest.html
    // https://github.com/uNetworking/uWebSockets/issues/805
    std::string buffer;
    /* Move it to storage of lambda */
    res->onData([this, res, buffer = std::move(buffer), dealer = std::move(dealer)](std::string_view piece, bool last) mutable {
        /* Mutate the captured data */
        buffer.append(piece.data(), piece.length());
        if (last)
        {
            dealer(buffer);
        }
    });
}
void HttpHome::handle_sql(auto *res, auto *req)
{
    get_post_data(res, [this, response = res](string payload) {        
        try
        {
            response->writeHeader("Content-Type", "application/json;charset=utf-8");
            response->writeHeader("Connection", "keep-alive");
            response->writeHeader("Access-Control-Allow-Origin", "*");
            // throw exception hurt performance, so try to not use: try...catch...throw
            auto data = Json::parse(payload);
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
                response->end(db_->exec_sql(sql));
            }
            else
            if(pass0 == pass) 
            {
                response->end(db_->exec_sql(sql));
            }
            else 
            {
                Json res;
                res["ret"] = -1;
                res["msg"] = "permission denied";
                response->end(res.dump());
            }
        }
        catch (const exception &e)
        {
            Json res;
            res["ret"] = -1;
            res["msg"] = e.what();
            res_json(response, res);
        }   
    });
}
void HttpHome::res_json(auto *res, Json &data)
{
    res->writeHeader("Content-Type", "application/json;charset=utf-8");
    res->writeHeader("Connection", "keep-alive");
    res->end(data.dump());
}
void HttpHome::handle_cors(auto *response, auto *req)
{
    response->writeHeader("Content-Type", "text/plain; charset=utf-8");
    response->writeHeader("Access-Control-Allow-Origin", "*");
    response->writeHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE");
    response->writeHeader("Access-Control-Max-Age", "1728000");
    response->writeHeader("Access-Control-Allow-Headers", "Authorization, Origin, X-Requested-With, Content-Type, Accept");
    response->end("");
}
void HttpHome::login(auto *res, auto *req)
{
    get_post_data(res, [this, response=res](string payload) {
        Json res;
        try
        {
            auto data = Json::parse(payload);
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
    });
}
void HttpHome::check_pass(auto *res, auto *req)
{
    get_post_data(res, [this, response=res](string payload) {
        Json res;
        try
        {
            auto data = Json::parse(payload);
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
    });
}

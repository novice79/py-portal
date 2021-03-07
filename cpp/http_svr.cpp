
#include "http_svr.h"

#include "util.h"

#include "ffmpeg.h"
using namespace std;

HttpSvr::HttpSvr(int port)
:port_(port)
{
    server_ = make_shared<HttpServer>();
    server_->config.port = port;
    ffmpeg_ = make_shared<FFmpeg>("ffmpeg", this);
    init();
}
HttpSvr::~HttpSvr()
{
    server_->stop();
}
void HttpSvr::ws_to_all(const std::string &json)
{
    server_->io_service->post([this, json] {
        ws_svr_.to_all(json);
    });
}
void HttpSvr::res_json(std::shared_ptr<HttpServer::Response> response, Json &data)
{
    SimpleWeb::CaseInsensitiveMultimap header;
    header.emplace("Content-Type", "application/json;charset=utf-8");
    header.emplace("Connection", "keep-alive");
    response->write(data.dump(), header);
}
void HttpSvr::init()
{    
    handle_upload();
    handle_upload_home();
    serve_res();
    spa_app();
    get_files();
    emplace_ws();
    client_info();
    ffmpeg();
    uncompress();
    start_node();
    // this block
    server_->start();
    static boost::asio::deadline_timer routine_timer(*server_->io_service, boost::posix_time::seconds(1));
    // boost::asio::placeholders::error needs to be replaced with std::placeholders::_1
    // otherwise can not use std::bind( nedd boost::bind)
    routine_timer.async_wait( std::bind(&HttpSvr::routine, this, ph::_1, &routine_timer) );
}

void HttpSvr::routine(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t)
{
    // do something here
    LOGD("doing routine jobs ...");
    t->expires_at(t->expires_at() + boost::posix_time::seconds(2));
    t->async_wait(std::bind(&HttpSvr::routine, this, ph::_1, t));
}
void HttpSvr::emplace_ws()
{
    auto &ws_server = ws_svr_.get_ws_svr();
    server_->on_upgrade = [&ws_server](unique_ptr<SimpleWeb::HTTP> &socket, shared_ptr<HttpServer::Request> request) {
        auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
        connection->method = std::move(request->method);
        connection->path = std::move(request->path);
        connection->http_version = std::move(request->http_version);
        connection->header = std::move(request->header);
        ws_server.upgrade(connection);
    };
}

void HttpSvr::spa_app()
{
    server_->default_resource["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            fs::path web_root_path{ AP::instance().spa_path() };
            auto path = web_root_path / request->path;
            // Check if path is within web_root_path
            // if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
            //     !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
            //     throw invalid_argument("path must be within root path");

            if (fs::is_directory(path))
            {
                path /= "index.html";
            }
            else 
            if( !fs::exists(path) )
            {
                // This is for spa routing
                path = web_root_path / "index.html";
            }    
            if( !fs::exists(path) )
            {
                throw invalid_argument("index.html not exist");
            }     

            SimpleWeb::CaseInsensitiveMultimap header;
            if( Util::is_pac(path.string()) )
            {
                header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
            }
            else
            {
                header.emplace("Cache-Control", "max-age=86400");
            }
            
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
                        static vector<char> buffer(131072); // Safe when server is running on one thread
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
                                        cerr << "Connection interrupted" << endl;
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
void HttpSvr::get_files()
{
    server_->resource["^/get_files$"]["POST"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try {
            auto data = Json::parse(request->content);  
            auto path = data["path"].get<string>();
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json;charset=utf-8");
            header.emplace("Connection", "keep-alive");
            response->write( Util::get_files_json(path), header );
        }
        catch(const exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
        }
    };
}
void HttpSvr::uncompress()
{
    server_->resource["^/uncompress$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try {
            auto data = Json::parse(request->content);    
            // this file path is relative to mystore
            auto file = data["file"].get<string>();
            thread work_thread([this, file, response] {   
                fs::path full_path = AP::instance().full_store_path(file);
                auto to_dir = full_path.parent_path();      
                Json res;       
                res["ret"] = -1;  
                try
                {
                    int ret = Util::uncompress(full_path.string(), to_dir.string());                
                    if( 0 == ret )
                    {
                        res["ret"] = 0;
                        // response->write(SimpleWeb::StatusCode::success_ok);
                        ws_to_all( Util::refresh_files_noty() );
                    }
                    else
                    {                       
                        res["msg"] = "uncompress failed";  
                        // client expect to return json
                        // response->write(SimpleWeb::StatusCode::server_error_internal_server_error);
                    }                   
                }                                           
                catch (const exception &e)
                {
                    res["msg"] = e.what();
                }
                res_json(response, res);
            });
            work_thread.detach();
        }
        catch(const exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
        }
    };
}
void HttpSvr::ffmpeg()
{
    server_->resource["^/ffmpeg$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {       
        try {
            auto data = Json::parse(request->content);                   
            thread work_thread([this, data, response] {   
                auto code = data["code"].get<int>();
                Json res;   
// code: 0=get media streams; 1=srt file to vtt; 2= subtitle track to vtt; 
// 3=audio track to mp3 file; 4=video track to x264 with all other tracks                                       
                try
                {
                    res["ret"] = 0;
                    switch (code)
                    {
                    case 0:
                        {
                            auto media = data["media"].get<string>();
                            media = AP::instance().full_store_path(media);
                            res["streams"] = ffmpeg_->get_media_streams(media);
                        }
                        break;
                    case 1:
                        {
                            auto srt = data["srt"].get<string>();
                            srt = AP::instance().full_store_path(srt);
                            res["ret"] = ffmpeg_->srt_to_vtt(srt);
                            if( 0 == res["ret"] )
                            {
                                ws_to_all( Util::refresh_files_noty() );
                            }
                        }
                        break;   
                    case 2:
                        {
                            auto video = data["video"].get<string>();
                            video = AP::instance().full_store_path(video);
                            auto sid = data["sid"].get<string>();
                            auto lang = data["lang"].get<string>();
                            res["ret"] = ffmpeg_->sub_stream_to_vtt(video, sid, lang);
                            if( 0 == res["ret"] )
                            {
                                ws_to_all( Util::refresh_files_noty() );
                            }
                        }
                        break; 
                    case 3:
                        {
                            auto video = data["video"].get<string>();
                            auto sid = data["sid"].get<string>();
                            auto lang = data["lang"].get<string>();
                            res_json(response, res);
                            // LOGI("before call ffmpeg_->audio_stream_to_mp3(%s, %s, %s)", video.c_str(), sid.c_str(), lang.c_str() );
                            ffmpeg_->audio_stream_to_mp3(video, sid, lang);
                        }
                        return;        
                    case 4:
                        {
                            auto video = data["video"].get<string>();
                            res_json(response, res);
                            // LOGI("before call ffmpeg_->convert_to_x264(%s)", video.c_str() );
                            ffmpeg_->convert_to_x264(video);
                        }
                        return;   
                    default:
                        break;
                    }      
                    res_json(response, res);            
                }                                           
                catch (const exception &e)
                {
                    res["ret"] = -1;
                    res["msg"] = e.what();
                    res_json(response, res);
                }               
            });
            work_thread.detach();
        }
        catch(const exception &e) {
            Json res;
            response->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
        }
    };
}
void HttpSvr::client_info()
{
    server_->resource["^/info$"]["GET"] = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    stringstream stream;
    stream << "<h1>Request from " << request->remote_endpoint().address().to_string() << ":" << request->remote_endpoint().port() << "</h1>";

    stream << request->method << " " << request->path << " HTTP/" << request->http_version;

    stream << "<h2>Query Fields</h2>";
    auto query_fields = request->parse_query_string();
    for(auto &field : query_fields)
      stream << field.first << ": " << field.second << "<br>";

    stream << "<h2>Header Fields</h2>";
    for(auto &field : request->header)
      stream << field.first << ": " << field.second << "<br>";

    response->write(stream);
  };
}
void HttpSvr::handle_upload()
{
    server_->resource["^/upload$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            const string& data = request->content.string();
            // LOGD( Util::hexStr(data) );
            const string file_name{ data.c_str() };
            // LOGD("file_name: %s", file_name.c_str());
            auto flag = static_cast<int>(data[data.length() - 1]);
            // cout<< "flag=" <<flag <<endl;
            auto buff_len = data.length() - file_name.length() -2;
            // cout<< "buff_len=" <<buff_len <<endl;
            const string& buff = data.substr(file_name.length()+1, buff_len);
            const auto path = AP::instance().store_path() + "/" + file_name;
            // cout<< "buff=" <<buff <<endl;
            std::shared_ptr<std::ofstream> writer;
            switch(flag)
            {
                case 0:
                    writer = writers_[file_name] = make_shared<std::ofstream>(path, std::ofstream::binary);
                    break;
                case 1:
                    writer = writers_[file_name];
                    break;
                case 2:
                    auto it = writers_.find(file_name);
                    if( it != writers_.end() )
                    {   
                        writer = it->second;
                        writers_.erase(it);
                    }
                    else 
                    {
                        writer = make_shared<ofstream>(path, std::ofstream::binary);
                    }
                    // relative to mystore path
                    ws_to_all( Util::get_files_json( "/" ) );
                    break;
            }
            writer->write( buff.c_str(), buff.length() );
            // cout << Util::string_to_hex( data ) << endl; 

            string rs = "ok";
            *response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: text/html; charset=utf-8\r\n"
                      << "Content-Length: " << rs.length() << "\r\n\r\n"
                      << rs;
        }
        catch (const exception &e)
        {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                      << e.what();
        }
    };
}
void HttpSvr::read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs, size_t len)
{
    if(len <= 0) return;
    // Read and send 128 KB at a time
    static const size_t buff_len = 131072;
    static vector<char> buffer(buff_len); // Safe when server is running on one thread
    size_t read_len = std::min(len, buff_len);
    streamsize read_length;
    if ((read_length = ifs->read( &buffer[0], static_cast<streamsize>(read_len) ).gcount()) > 0)
    {
        response->write(&buffer[0], read_length);
        // cout << "write buff len = " << read_length << endl;
        response->send([=](const SimpleWeb::error_code &ec) {
            if (!ec)
                read_and_send(response, ifs, len - read_length);
            else
                cerr << "Connection interrupted: " << ec.message() << endl;
        });  
    }
}
void HttpSvr::serve_res()
{
    server_->resource["^/store/(.+)$"]["GET"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            auto fn = request->path_match[1].str();
            fn = Util::urlDecode(fn);
            fs::path store_root_path{AP::instance().store_path()};
            auto path = store_root_path / fn;
            SimpleWeb::CaseInsensitiveMultimap header;
            // enable Cache-Control, except pac file
            if( Util::is_pac(path.string()) )
            {
                header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
            }
            else
            {
                header.emplace("Cache-Control", "max-age=86400");
            }
            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs)
            {
                header.emplace("Content-Type", Util::mime_type(path.string()) );
                size_t file_len = ifs->tellg();
                ifs->seekg(0, ios::beg);
                size_t length = file_len;
                auto it = request->header.find("Range");
                if(it != request->header.end()) {
                    auto range = it->second;
                    boost::replace_all(range, "bytes=", "");
                    auto vs = Util::split(range, "-");
                    size_t begin = stoul( vs[0] );
                    size_t end = vs[1] == "" ? file_len : stoul( vs[1] );
                    // [begin, end] not [begin, end)
                    end = std::min( file_len - 1, end );
                    length = (end - begin) + 1;
                    range = "bytes " + vs[0] + "-" + to_string(end) + "/" + to_string(file_len);
                    header.emplace("Content-Range", range); 
                    header.emplace("Accept-Ranges", "bytes");
                    header.emplace("Content-Length", to_string(length));
                    response->write(SimpleWeb::StatusCode::success_partial_content, header);
                    ifs->seekg(begin, ios::beg);
                } 
                else 
                {
                    header.emplace("Content-Length", to_string(length));
                    response->write(header);
                }
                read_and_send(response, ifs, length);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e)
        {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path_match[1].str() + ": " + e.what());
        }
    };
}
void HttpSvr::handle_upload_home()
{
    server_->resource["^/upload_home$"]["POST"] = [this](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        try
        {
            const string& data = request->content.string();
            // LOGD( Util::hexStr(data) );
            const string file_name{ data.c_str() };
            // LOGD("file_name: %s", file_name.c_str());
            auto flag = static_cast<int>(data[data.length() - 1]);
            // cout<< "flag=" <<flag <<endl;
            auto buff_len = data.length() - file_name.length() -2;
            // cout<< "buff_len=" <<buff_len <<endl;
            const string& buff = data.substr(file_name.length()+1, buff_len);
            auto path = AP::instance().store_path() + "/home/";
            if( !fs::exists(path) )
            {
                fs::create_directory(path);
            }
            path += file_name;
            // cout<< "buff=" <<buff <<endl;
            std::shared_ptr<std::ofstream> writer;
            switch(flag)
            {
                case 0:
                    writer = writers_[file_name] = make_shared<std::ofstream>(path, std::ofstream::binary);
                    break;
                case 1:
                    writer = writers_[file_name];
                    break;
                case 2:
                    auto it = writers_.find(file_name);
                    if( it != writers_.end() )
                    {   
                        writer = it->second;
                        writers_.erase(it);
                    }
                    else 
                    {
                        writer = make_shared<ofstream>(path, std::ofstream::binary);
                    }
                    ws_to_all( Util::get_files_json( "/" ) );
                    break;
            }
            writer->write( buff.c_str(), buff.length() );
            // cout << Util::string_to_hex( data ) << endl; 
            if(2 == flag)
            {
                writer->close();
                string to_dir = AP::instance().home_path();
                Util::uncompress(path, to_dir);
                if( fs::exists(to_dir+"/node_modules/sqlite3") )
                {
                    Util::uncompress(AP::instance().prefab_path() + "/sqlite3.7z", to_dir+"/node_modules");
                }                
                // LOGI("extractZip=%s to %s", path.c_str(), to_dir.c_str() );
                start_node();
            }
            string rs = "ok";
            *response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: text/html; charset=utf-8\r\n"
                      << "Content-Length: " << rs.length() << "\r\n\r\n"
                      << rs;
        }
        catch (const exception &e)
        {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                      << e.what();
        }
    };
}
void HttpSvr::start_node()
{
    if( node_process_ )
    {
        node_process_->terminate();                
    }
    auto cmd = "node " + AP::instance().home_path() + "/app.js";
    node_process_ = make_shared<bp::child>( cmd );
    LOGI("rerun node: %1%", cmd );
}
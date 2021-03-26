
#include "http_svr.h"

#include "util.h"

#include "ffmpeg.h"
using namespace std;

HttpSvr::HttpSvr(int port)
    : port_(port), app_(std::make_shared<uWS::App>())
{
    ffmpeg_ = make_shared<FFmpeg>("ffmpeg", this);
    init_handlers();
}
HttpSvr::~HttpSvr()
{
}
void HttpSvr::init_handlers()
{
    // handlers_["get_files"] = [](Json &json, WSType* ws) {
    //     const string &path = json["path"].get<std::string>();
    //     ws->send( Util::get_files_json(path) );
    // };
}
void HttpSvr::ws_to_all(std::string json)
{
    loop_->defer([this, json=move(json)]() {
        // LOG("broadcast %1%", json)
        app_->publish("broadcast", json, uWS::OpCode::TEXT);
    });
}
void HttpSvr::res_json_str(auto *res, std::string data)
{
    res->writeHeader("Content-Type", "application/json;charset=utf-8");
    res->writeHeader("Connection", "keep-alive");
    res->end(data);
}
void HttpSvr::res_json(auto *res, Json &data)
{
    res_json_str(res, data.dump());
}
void HttpSvr::run()
{
    loop_ = uWS::Loop::get();
    start_node();
    app_->get("/*", [this](auto *res, auto *req) {
            serve_app(res, req);
        })
        .get("/cli_info/*", [this](auto *res, auto *req) {
            cli_info(res, req);
            res->onAborted([]() {LOG("cli_info aborted")});
        })
        .get("/store/*", [this](auto *res, auto *req) {
            serve_res(res, req);
            res->onAborted([]() {LOG("serve_res aborted")});
        })
        .post("/get_files", [this](auto *res, auto *req) {
            get_files(res, req);
            res->onAborted([]() {LOG("get_files aborted")});
        })
        .post("/file_op/:type", [this](auto *res, auto *req) {
            file_op(res, req);
            res->onAborted([]() {LOG("file_op aborted")});
        })
        .post("/uncompress", [this](auto *res, auto *req) {
            uncompress(res, req);
            res->onAborted([]() {LOG("uncompress aborted")});
        })
        .post("/upload", [this](auto *res, auto *req) {
            handle_upload(res, req);
            res->onAborted([]() {LOG("upload aborted")});
        })
        .post("/upload_home", [this](auto *res, auto *req) {
            handle_upload_home(res, req);
            res->onAborted([]() {LOG("upload_home aborted")});
        })
        .post("/ffmpeg", [this](auto *res, auto *req) {
            ffmpeg(res, req);
            res->onAborted([]() {LOG("ffmpeg aborted")});
        })
        .ws<PerSocketData>("/cpp_channel",  {/* Settings */
        .maxPayloadLength = 16 * 1024 * 1024,
        .idleTimeout = 16,
        .maxBackpressure = 1 * 1024 * 1024,
        .closeOnBackpressureLimit = false,
        .sendPingsAutomatically = true,
        /* Handlers */
        .upgrade = nullptr,
        .open = [this](auto *ws) {
            LOG("%1% opened ws", string{ws->getRemoteAddressAsText()})
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
            ws->subscribe("broadcast"); 
        },
        .message = [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
            // cout<<"ws get msg="<<message << "; opCode=" << opCode<<endl;
            if(message == "") return;
            try
            {
                auto data = Json::parse(message);    
                auto cmd = data["cmd"].get<string>();
                auto it = handlers_.find(cmd);
                if (it != handlers_.end())
                    return it->second(data, ws);
                data["ret"] = -1;
                using boost::format;
                data["msg"] = str(format( R"foo(can not find command dealer of %1%")foo") % cmd);
                ws->send(data.dump());
            }
            catch(const std::exception& e)
            {
                LOGE("ws onmessage throw exception: %s", e.what());
                Json json;
                json["ret"] = -1;
                json["msg"] = e.what();
                ws->send(json.dump());
            }
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
        .close = [this](auto * ws, int /*code*/, std::string_view /*message*/) {
            /* You may access ws->getUserData() here */
            LOG("%1% closed ws", string{ws->getRemoteAddressAsText()})
        }
        }).listen("0.0.0.0", port_, [this](auto *token) {
            if (token)
            {
                LOG("file server listen at %1%", port_)
            }
        }).run();
}
void HttpSvr::cli_info(auto *res, auto *req)
{
    res->writeHeader("Content-Type", "text/html");
    res->write( boost::str(boost::format("<h1>Request from: <i>%1%</i></h1>") % res->getRemoteAddressAsText() ) );
    res->write( "<h2>Query Fields</h2>" );
    res->write( req->getQuery() );
    res->write( "<h2>Header Fields</h2>" );
    for(auto i = req->begin(); i != req->end(); ++i)
    {
        auto h = boost::format("<div>%1%: %2%</div>") % (*i).first % (*i).second ;
        res->write( boost::str(h) );
    }
    res->end();
}
void HttpSvr::serve_app(auto *res, auto *req)
{
    try
    {
        fs::path web_root_path{AP::instance().spa_path()};
        auto path = web_root_path / string(req->getUrl());
        if (fs::is_directory(path))
        {
            path /= "index.html";
        }
        else if (!fs::exists(path))
        {
            // This is for spa routing
            path = web_root_path / "index.html";
        }
        if (!fs::exists(path))
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
void HttpSvr::get_post_data(auto *res, std::function<void(std::string)> dealer)
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
void HttpSvr::get_files(auto *res, auto *req)
{
    get_post_data(res, [this, res](string payload) {
        try
        {
            auto data = Json::parse(payload);
            auto path = data["path"].get<string>();
            res_json_str(res, Util::get_files_json(path));
        }
        catch (const exception &e)
        {
            res->writeStatus("400 Bad Request");
            res->end(e.what());
        }
    });
}
void HttpSvr::file_op(auto *res, auto *req)
{
    get_post_data(res, [this, res, req](string payload) {
        Json rj;
        rj["ret"] = -1;
        try
        {   
            auto type = req->getParameter(0);
            auto data = Json::parse(payload);
            if(type == "delete")
            {
                // can delete multiple files
                std::vector<std::string> files = data["files"];
                int count = 0;
                string path;
                for (auto f : files)
                {
                    path =  AP::instance().full_store_path(f);
                    auto c = fs::remove_all(path);
                    if(c > 0)
                    {
                        LOGI("c++ removed %s", path.c_str());
                        ++count;
                    }
                    else
                    {
                        LOGI("c++ remove %s failed", path.c_str());
                    }
                }
                // assume all these files are in the same dir, so only need notify once 
                if(count > 0)
                {
                    rj["ret"] = 0;
                    rj["count"] = count;
                    ws_to_all( Util::refresh_files_noty(path) );
                }
                // if fail will throw               
            }
            else
            if(type == "rename")
            {
                std::vector<std::string> files = data["old_names"];
                string new_name = data["new_name"].get<std::string>();
                new_name = AP::instance().full_store_path(new_name);
                int count = 0;
                string from, to;
                for (auto f : files)
                {
                    from =  AP::instance().full_store_path(f);
                    if( fs::is_directory(new_name) )
                    {
                        to = (fs::path(new_name) / fs::path(from).filename()).string();
                    }
                    else
                    {
                        to = new_name;
                    }
                    LOGI("c++ rename %s to %s", from.c_str(), to.c_str());
                    fs::rename(from, to);             
                    ++count;
                }
                // assume all these files are in the same dir, so only need notify once 
                if(count > 0)
                {
                    rj["ret"] = 0;
                    rj["count"] = count;
                    ws_to_all( Util::refresh_files_noty(from) );
                }
            }
            else
            if(type == "create_dir")
            {
                string path = data["path"].get<std::string>();
                path = AP::instance().full_store_path(path);
                LOGI("c++ create dir %s", path.c_str());
                fs::create_directories(path);
                ws_to_all( Util::refresh_files_noty(path) );
                rj["ret"] = 0;
            }
            res_json(res, rj);
        }
        catch (const exception &e)
        {
            res->writeStatus("400 Bad Request");
            rj["msg"] = e.what();
            res_json(res, rj);
        }
    });
}
void HttpSvr::uncompress(auto *res, auto *req)
{
    get_post_data(res, [this, res](string payload) {
        try
        {
            auto data = Json::parse(payload);
            // this file path is relative to mystore
            auto file = data["file"].get<string>();
            thread work_thread([this, file, res] {
                fs::path full_path = AP::instance().full_store_path(file);
                auto to_dir = full_path.parent_path();
                Json rd;
                rd["ret"] = -1;
                try
                {
                    int ret = Util::uncompress(full_path.string(), to_dir.string());
                    if (0 == ret)
                    {
                        rd["ret"] = 0;
                        // response->write(SimpleWeb::StatusCode::success_ok);
                        ws_to_all(Util::refresh_files_noty(full_path.string()));
                    }
                    else
                    {
                        rd["msg"] = "uncompress failed";
                        // client expect to return json
                        // response->write(SimpleWeb::StatusCode::server_error_internal_server_error);
                    }
                }
                catch (const exception &e)
                {
                    rd["msg"] = e.what();
                }
                loop_->defer([this, res, json_str=rd.dump()]() {
                    res_json_str(res, json_str);
                });
            });
            work_thread.detach();
        }
        catch (const exception &e)
        {
            res->writeHeader("Content-Type", "text/plain");
            res->writeStatus("400 Bad Request");
            res->end(e.what());
        }
    });
}
void HttpSvr::ffmpeg(auto *res, auto *req)
{
    get_post_data(res, [this, response = res](string payload) {
        try
        {
            auto data = Json::parse(payload);
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
                        if (0 == res["ret"])
                        {
                            ws_to_all(Util::refresh_files_noty(srt));
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
                        if (0 == res["ret"])
                        {
                            ws_to_all(Util::refresh_files_noty(video));
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
        catch (const exception &e)
        {
            Json res;
            res["ret"] = -1;
            res["msg"] = e.what();
            res_json(response, res);
        }
    });
}

void HttpSvr::handle_upload(auto *res, auto *req)
{
    get_post_data(res, [this, res](string payload) {
        res->writeHeader("Content-Type", "text/plain");
        try
        {
            const string &data = payload;
            // LOGD( Util::hexStr(data) );
            const string file_name{data.c_str()};
            // LOGD("file_name: %s", file_name.c_str());
            auto flag = static_cast<int>(data[data.length() - 1]);
            // cout<< "flag=" <<flag <<endl;
            auto buff_len = data.length() - file_name.length() - 2;
            // cout<< "buff_len=" <<buff_len <<endl;
            const string &buff = data.substr(file_name.length() + 1, buff_len);
            const auto path = AP::instance().store_path() + "/" + file_name;
            // cout<< "buff=" <<buff <<endl;
            std::shared_ptr<std::ofstream> writer;
            switch (flag)
            {
            case 0:
                writer = writers_[file_name] = make_shared<std::ofstream>(path, std::ofstream::binary);
                break;
            case 1:
                writer = writers_[file_name];
                break;
            case 2:
                auto it = writers_.find(file_name);
                if (it != writers_.end())
                {
                    writer = it->second;
                    writers_.erase(it);
                }
                else
                {
                    writer = make_shared<ofstream>(path, std::ofstream::binary);
                }
                // relative to mystore path
                ws_to_all(Util::refresh_files_noty(""));
                break;
            }
            writer->write(buff.c_str(), buff.length());
            // cout << Util::string_to_hex( data ) << endl;
            res->end("ok");
        }
        catch (const exception &e)
        {
            // or "500 Internal Server Error"
            res->writeStatus("400 Bad Request");
            res->end(e.what());
        }
    });
}

void HttpSvr::serve_res(auto *res, auto *req)
{
    try
    {
        auto fn = string{req->getUrl()};
        static const string prefix = "/store/";
        fn = fn.substr( prefix.length() );
        // LOG("in serve_res, fn=%1%", fn)
        fn = Util::urlDecode(fn);
        fs::path store_root_path{AP::instance().store_path()};
        auto path = store_root_path / fn;
        // LOG("get store file %1%", path.string())
        make_shared<AsyncFileStreamer>(path.string(), res, req)->streamFile();
    }
    catch (const exception &e)
    {
        LOG("exception:%1%", e.what() )
        res->writeStatus("400 Bad Request");
        res->end(e.what());
    }
}
void HttpSvr::handle_upload_home(auto *res, auto *req)
{
    get_post_data(res, [this, res](string payload) {
        res->writeHeader("Content-Type", "text/plain");
        try
        {
            const string &data = payload;
            // LOGD( Util::hexStr(data) );
            const string file_name{data.c_str()};
            // LOGD("file_name: %s", file_name.c_str());
            auto flag = static_cast<int>(data[data.length() - 1]);
            // cout<< "flag=" <<flag <<endl;
            auto buff_len = data.length() - file_name.length() - 2;
            // cout<< "buff_len=" <<buff_len <<endl;
            const string &buff = data.substr(file_name.length() + 1, buff_len);
            auto path = AP::instance().store_path() + "/home/";
            if (!fs::exists(path))
            {
                fs::create_directory(path);
            }
            path += file_name;
            // cout<< "buff=" <<buff <<endl;
            std::shared_ptr<std::ofstream> writer;
            switch (flag)
            {
            case 0:
                writer = writers_[file_name] = make_shared<std::ofstream>(path, std::ofstream::binary);
                break;
            case 1:
                writer = writers_[file_name];
                break;
            case 2:
                auto it = writers_.find(file_name);
                if (it != writers_.end())
                {
                    writer = it->second;
                    writers_.erase(it);
                }
                else
                {
                    writer = make_shared<ofstream>(path, std::ofstream::binary);
                }
                ws_to_all(Util::refresh_files_noty(""));
                break;
            }
            writer->write(buff.c_str(), buff.length());
            // cout << Util::string_to_hex( data ) << endl;
            if (2 == flag)
            {
                writer->close();
                string to_dir = AP::instance().home_path();
                Util::uncompress(path, to_dir);
                if (fs::exists(to_dir + "/node_modules/sqlite3"))
                {
                    Util::uncompress(AP::instance().prefab_path() + "/sqlite3.7z", to_dir + "/node_modules");
                }
                if (fs::exists(to_dir + "/node_modules/better-sqlite3"))
                {
                    Util::uncompress(AP::instance().prefab_path() + "/better-sqlite3.7z", to_dir + "/node_modules");
                }
                // LOGI("extractZip=%s to %s", path.c_str(), to_dir.c_str() );
                start_node();
            }
            res->end("ok");
        }
        catch (const exception &e)
        {
            res->writeStatus("400 Bad Request");
            res->end(e.what());
        }
    });
}
void HttpSvr::start_node()
{
    if (bp::search_path("node").empty())
        return;
    if (node_process_)
    {
        node_process_->terminate();
    }
    auto cmd = "node " + AP::instance().home_path() + "/app.js";
    node_process_ = make_shared<bp::child>(cmd);
    LOGI("rerun node: %1%", cmd);
}
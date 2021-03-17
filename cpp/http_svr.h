#pragma once
#include "common.h"
#include "async_file_streamer.h"

class FFmpeg;
class HttpSvr
{
    typedef struct uWS::WebSocket<false, true, PerSocketData> WSType;
    typedef std::function<void(Json&, WSType*)> Handlers;
    std::map<std::string, Handlers> handlers_;
    std::shared_ptr<FFmpeg> ffmpeg_;
    std::map<std::string, std::shared_ptr<std::ofstream>> writers_;
    std::shared_ptr<bp::child> node_process_;
    uWS::Loop *loop_;
    std::shared_ptr<uWS::App> app_;
    int port_;
public:
    HttpSvr(int port);
    ~HttpSvr();
    void run();
    void init_handlers();
    void ws_to_all(std::string json);
    int get_port(){return port_;}
    void res_json(auto *res, Json& data);
    void res_json_str(auto *res, std::string data);
private:
    void start_node();
    void get_post_data(auto *res, std::function<void (std::string)>);
    void cli_info(auto *res, auto *req);
    void serve_app(auto *res, auto *req);
    void serve_res(auto *res, auto *req);
    void get_files(auto *res, auto *req);
    void handle_upload(auto *res, auto *req);
    void handle_upload_home(auto *res, auto *req);
    void uncompress(auto *res, auto *req);

    void ffmpeg(auto *res, auto *req);

};
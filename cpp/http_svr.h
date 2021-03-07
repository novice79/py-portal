#pragma once
#include "common.h"
// #include "server_ws.hpp"
#include "server_http.hpp"
#include "ws_svr.h"
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
// typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

class FFmpeg;
class HttpSvr
{
    std::shared_ptr<FFmpeg> ffmpeg_;
    std::shared_ptr<HttpServer> server_;
    WsSvr ws_svr_;
    std::map<std::string, std::shared_ptr<std::ofstream>> writers_;
    std::shared_ptr<bp::child> node_process_;
    int port_;
public:
    HttpSvr(int port);
    ~HttpSvr();
    void init();
    void ws_to_all(const std::string& json);
    int get_port(){return port_;}
    void res_json(std::shared_ptr<HttpServer::Response> response, Json& data);
private:
    void start_node();
    void spa_app();
    void serve_res();
    void get_files();
    void handle_upload();
    void handle_upload_home();
    void uncompress();
    void client_info();
    void ffmpeg();
    void emplace_ws();
    void read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<std::ifstream> &ifs, size_t len);
    void routine(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t);
};
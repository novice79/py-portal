#pragma once
#include "common.h"
#include "db.h"
#include "server_http.hpp"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class HttpHome
{
    WsServer ws_server_;
    std::shared_ptr<HttpServer> server_;
    DB* db_;
    int port_;
public:
    HttpHome(int port);
    ~HttpHome();
    void init();
    int get_port(){return port_;}
private:
    void static_dir();
    void handle_sql();
    void login();
    void handle_cors();
    void check_pass();
    void emplace_ws();
    void res_json(std::shared_ptr<HttpServer::Response> response, Json& data);
};
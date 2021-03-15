#pragma once
#include "common.h"
#include "db.h"

#include "async_file_streamer.h"
class HttpHome
{
    DB* db_;
    int port_;
    uWS::Loop *loop_;
    std::shared_ptr<uWS::App> app_;
public:
    HttpHome(int port);

    void run();
    int get_port(){return port_;}
private:
    void static_dir(auto *res, auto *req);
    void get_post_data(auto *res, std::function<void(std::string)> dealer);
    void handle_sql(auto *res, auto *req);
    void login(auto *res, auto *req);
    void handle_cors(auto *res, auto *req);
    void check_pass(auto *res, auto *req);
    void res_json(auto *res, Json& data);
};
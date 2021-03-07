#include "ws_svr.h"
#include "util.h"

using namespace std;
using boost::format;
WsSvr::WsSvr()
{
    init();
}
void WsSvr::init()
{
    handlers_["del_file"] = [this](Json &json, std::shared_ptr<WsServer::Connection> cnn) {
        string path = json["path"].get<std::string>();
        path = AP::instance().full_store_path(path);
        LOGI("c++ remove %s", path.c_str());
        fs::remove_all(path);
        to_all( Util::refresh_files_noty() );
        json["ret"] = 0;
        return_json(cnn, json);
    };
    handlers_["rename_file"] = [this](Json &json, std::shared_ptr<WsServer::Connection> cnn) {
        string path = json["path"].get<std::string>();
        path = AP::instance().full_store_path(path);
        string new_name = json["new_name"].get<std::string>();
        new_name = AP::instance().full_store_path(new_name);
        LOGI("c++ rename %s to %s", path.c_str(), new_name.c_str());
        fs::rename(path, new_name);
        to_all( Util::refresh_files_noty() );
        json["ret"] = 0;
        return_json(cnn, json);
    };
    handlers_["create_dir"] = [this](Json &json, std::shared_ptr<WsServer::Connection> cnn) {
        string path = json["path"].get<std::string>();
        path = AP::instance().full_store_path(path);
        LOGI("c++ create dir %s", path.c_str());
        fs::create_directories(path);
        to_all( Util::refresh_files_noty() );
        json["ret"] = 0;
        return_json(cnn, json);
    };
    handlers_["get_files"] = [](Json &json, std::shared_ptr<WsServer::Connection> cnn) {
        const string &path = json["path"].get<std::string>();
        cnn->send( Util::get_files_json(path) );
    };
    ep_for_cpp();

}
void WsSvr::to_all(const std::string &json)
{
    auto &cpp_channel_endpoint = ws_server_.endpoint["^/cpp_channel/?$"];
    for (auto &a_connection : cpp_channel_endpoint.get_connections())
        a_connection->send(json);
}
void WsSvr::ep_for_cpp()
{
    auto &fs_endpoint = ws_server_.endpoint["^/cpp_channel/?$"];
    fs_endpoint.on_open = [&](shared_ptr<WsServer::Connection> connection) {
        // relative path to mystore
        auto out_message = make_shared<string>(Util::get_files_json( "/" ));
        connection->send(*out_message);
        cout << "Server: Opened connection " << connection.get() << endl;
    };

    fs_endpoint.on_close = [&](shared_ptr<WsServer::Connection> connection, int status, const string &reason) {
        cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
    };

    fs_endpoint.on_error = [&](shared_ptr<WsServer::Connection> connection, const boost::system::error_code &ec) {
        cerr << "Server: Error in connection " << connection.get() << ". "
             << "Error: " << ec << ", error message: " << ec.message() << endl;
    };
    fs_endpoint.on_message = [this](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
        auto cmd_json = in_message->string();
        if(cmd_json == "") return;
        try
        {
            auto data = Json::parse(cmd_json);    
            auto cmd = data["cmd"].get<string>();
            auto it = handlers_.find(cmd);
            if (it != handlers_.end())
                return it->second(data, connection);
            // cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << endl;
            // cout << "Server: Sending message \"" << out_message << "\" to " << connection.get() << endl;
            // connection->send is an asynchronous function
            data["ret"] = -1;
            data["msg"] = str(format( R"foo(can not find command dealer of %1%")foo") % cmd);
            return_json(connection, data);
        }
        catch(const std::exception& e)
        {
            LOGE("ws onmessage throw exception: %s", e.what());
            Json json;
            json["ret"] = -1;
            json["msg"] = e.what();
            return_json(connection, json);
        }              
    };
}
void WsSvr::return_json(std::shared_ptr<WsServer::Connection> cnn, Json& json)
{
    cnn->send(json.dump(), [](const SimpleWeb::error_code &ec) {
        if (ec)
        {
            cout << "Server: Error sending message. " <<
                // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                "Error: " << ec << ", error message: " << ec.message() << endl;
        }
    });
}

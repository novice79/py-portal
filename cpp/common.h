#pragma once

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <regex>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <unordered_set>
namespace ph = std::placeholders;

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/process.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/preprocessor.hpp>
#include <boost/vmd/vmd.hpp>
#include <boost/locale.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
namespace bp = boost::process;
namespace fs = boost::filesystem;

#include <nlohmann/json.hpp>
using Json = nlohmann::json;

#include "server_ws.hpp"
typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y) boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z) boost::bind(&self_type::x, shared_from_this(),y,z)
template<typename T>
class Singleton {
public:
    static T& instance()
    {
        static T* instance{0};
        if (!instance) 
        {
            instance = new T(); 
            instance->init();
        }
        return *instance;
    }
    Singleton(const Singleton&) = delete;
    Singleton& operator= (const Singleton) = delete;
protected:
    Singleton() {}
    virtual void init(){}
};
// use this without first {
#define ToSingle(T) \
class T: public Singleton<T> \
{ \
private: T(){} \
friend Singleton<T>; 

ToSingle(PY)
public:
    void init();
    boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;
private:
}; 
// AP stand for app path
ToSingle(AP)
    fs::path 
        exe_path_,
        spa_path_, 
        store_path_, 
        home_path_, 
        prefab_path_, 
        log_path_, 
        tmp_path_;
public:
    void initialize(std::string exe_path);
    std::string exe_path(){return exe_path_.string();}
    std::string spa_path(){return spa_path_.string();}
    std::string store_path(){return store_path_.string();}
    std::string rel_store_path(std::string full_path) 
    {
        return fs::relative(full_path, store_path_).string();
    }    
    std::string full_store_path(std::string rel_path) 
    {
        return (store_path_ / rel_path).string();
    }     
    std::string home_path(){return home_path_.string();}
    std::string prefab_path(){return prefab_path_.string();}
    std::string log(){return log_path_.string();}
    std::string tmp(){return tmp_path_.string();}
}; 
// AP::instance().store_path()
#define PY_TRACE BOOST_LOG_SEV(PY::instance().lg, boost::log::trivial::trace)
#define PY_DEBUG BOOST_LOG_SEV(PY::instance().lg, boost::log::trivial::debug)
#define PY_INFO  BOOST_LOG_SEV(PY::instance().lg, boost::log::trivial::info)
#define PY_WARN  BOOST_LOG_SEV(PY::instance().lg, boost::log::trivial::warning)
#define PY_ERROR BOOST_LOG_SEV(PY::instance().lg, boost::log::trivial::error)
#define PY_FATAL BOOST_LOG_SEV(PY::instance().lg, boost::log::trivial::fatal)
#define DATA(r, symbol, elem) symbol elem
#define BOOST_FORMAT(f, ...) boost::format(f) BOOST_PP_SEQ_FOR_EACH(DATA, % , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define LOGT(f, ...) \
PY_TRACE << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) ;
#define LOGD(f, ...) \
PY_DEBUG << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) ;
#define LOGI(f, ...) \
PY_INFO << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) ;
#define LOGW(f, ...) \
PY_WARN << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) ;
#define LOGE(f, ...) \
PY_ERROR << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) ;
#define LOGF(f, ...) \
PY_FATAL << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) ;

#define LOG(f, ...) \
std::cout << BOOST_PP_IF(BOOST_VMD_IS_EMPTY(__VA_ARGS__), f, BOOST_FORMAT(f, __VA_ARGS__)) << std::endl;

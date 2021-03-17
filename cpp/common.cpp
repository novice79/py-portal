
#include "common.h"

using namespace std;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void AP::initialize(std::string exe_path)
{
    fs::path ep{exe_path};
    exe_path_ = ep;
    spa_path_ = ep / "www";
    prefab_path_ = ep / "prefab";
    store_path_ = ep / "mystore";
    home_path_ = ep / "home";
    log_path_ = ep / "log";
    tmp_path_ = store_path_ / "tmp";
    // www and prefab must exist
    if( !fs::exists(store_path_) ) fs::create_directory(store_path_);
    if( !fs::exists(home_path_) ) 
        fs::copy(prefab_path_ / "home", home_path_, 
            fs::copy_options::overwrite_existing | fs::copy_options::recursive);
    if( !fs::exists(log_path_) ) fs::create_directory(log_path_);
    if( !fs::exists(tmp_path_) ) fs::create_directory(tmp_path_);
}
void PY::init()
{
#ifndef FOR_DOCKER
    cout<< "PY::init()" << endl;
    auto log_path = AP::instance().log();
    cout<< "log_path=" << log_path << endl;
    logging::add_file_log
    (
        keywords::file_name = log_path + "/py_%Y-%m-%d.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
		keywords::auto_flush = true,
        keywords::format = "[%TimeStamp%]:<%Severity%> %Message%",
        keywords::max_files = 10
    );
#endif
    // add console sink
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()) );

    // flush
    sink->locked_backend()->auto_flush(true);

    // format sink
    sink->set_formatter
    (
        expr::format("[%1%]: <%2%> %3%")
            % expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            % logging::trivial::severity
            % expr::smessage
        // expr::stream
        //     << "["
        //     << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
        //     << "]: <" << logging::trivial::severity
        //     << "> " << expr::smessage
    );
    // register sink
    logging::core::get()->add_sink(sink);

	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::trace
	);
	logging::add_common_attributes();
}



#include "common.h"
#include "util.h"
#include "http_svr.h"
#include "http_home.h"
using namespace std;

int main(int argc, char **argv)
{
  auto path = fs::canonical(fs::path(argv[0]).remove_filename());
  AP::instance().initialize( fs::system_complete(path).string() );
  int port = std::getenv("PORT") ? stoi( getenv("PORT") ) : 57000;
  fs::path exe = bp::search_path("minidlnad");
  if( !exe.empty() ) 
  {
    LOG("run minidlnad");
    bp::spawn(exe, "-v", "-d");
  }
  else 
  {
    LOG("minidlnad not exist");
  }
  exe = bp::search_path("transmission-daemon");
  if( !exe.empty() ) 
  {
    LOG("run transmission-daemon");
    bp::spawn("transmission-daemon -g /etc/transmission-daemon/");
  }
  else 
  {
    LOG("transmission-daemon not exist");
  }
  thread t([port]() {
    HttpHome home_svr(port + 1);
  });
  t.detach();
  LOG("11111111111111111111111111");
  HttpSvr server(port);
  return 0;
}

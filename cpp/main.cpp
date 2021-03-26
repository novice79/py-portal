
#include "common.h"
#include "util.h"
#include "http_svr.h"
#include "http_home.h"
#include "socks5.h"
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
    home_svr.run();
  });
  t.detach();

  thread t1([port]() {
    Socks5 socks(port + 1000);
    socks.run();
  });
  t1.detach();

  HttpSvr server(port);
  server.run();
  return 0;
}

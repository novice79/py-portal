
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
  thread t([port]() {
    HttpHome home_svr(port + 1);
  });
  t.detach();
  HttpSvr server(port);
  return 0;
}

#include "util.h"
#include "ffmpeg.h"
#include "http_svr.h"

using namespace std;
using boost::format;

FFmpeg::FFmpeg(const std::string& ff, HttpSvr *http_svr)
:ff_(ff),http_svr_(http_svr)
{

}
int FFmpeg::audio_stream_to_mp3(const std::string& video, std::string sid, std::string lang)
{
	string full_path = AP::instance().full_store_path(video);
	Json noty;
	noty["cmd"] = "ffmpeg_progress";
	int total_sec{0};
	get_video_duration_secs(full_path, total_sec);
	fs::path vp{full_path};
	bp::ipstream is; //reading pipe-stream
	string cmd = ff_ +
	" -y -hide_banner -i \"%1%\" "
	"-map \"%2%\" "
	"\"%3%\""
	;
	string fn = vp.stem().string() + "_" + lang + ".mp3";
	string out = vp.parent_path().string() + "/" + fn;
	cmd = str(format(cmd) % full_path % sid % out);
	LOGI("audio_stream_to_mp3 cmd=%s", cmd.c_str());
	bp::child c(cmd, bp::std_err > is);
	std::string line;
	std::regex r(R"(time=([^\s]+).+(speed=.+?x))");
    std::smatch sm;
    while (c.running() && std::getline(is, line, '\r') && !line.empty())
	{
		if( regex_search(line, sm, r) )
		{
			auto s = Util::dur_as_sec(sm[1]);			
			float p = static_cast<float>(s)/total_sec * 100;
			noty["progress"] = str(format("%.2f%%") % p);
			noty["speed"] = sm[2];
			noty["video"] = video;
			noty["sid"] = sid;
			http_svr_->ws_to_all( noty.dump() );			
			LOGI("提取mp3: %s%%--%s", noty["progress"].get<string>().c_str(), noty["speed"].get<string>().c_str() );
			// std::cout << "提取mp3: " << static_cast<float>(s)/total_sec * 100 << "%" << "--" << sm[2] <<'\n';
			// converted: 0.620939%--speed=3.12x
		}
	}
	c.wait();
	http_svr_->ws_to_all( Util::refresh_files_noty() );
    return c.exit_code();
}
int FFmpeg::srt_to_vtt(const std::string& srt)
{
	bp::ipstream is;
	string vtt = fs::path(srt).replace_extension(".vtt").string();
	string cmd = ff_ + " -i \"" + srt + "\" " + vtt;
	LOGI("srt_to_vtt cmd=%s", cmd.c_str());
	bp::child c(cmd, bp::std_err > is);
	c.wait();
    return c.exit_code();
}
int FFmpeg::sub_stream_to_vtt(const std::string& video, std::string sid, std::string lang)
{
	fs::path vp{video};
	bp::ipstream is; //reading pipe-stream
	string cmd = ff_ +
	" -y -hide_banner -i \"%1%\" "
	"-map \"%2%\" "
	"\"%3%\""
	;
	string fn = vp.stem().string() + "_" + lang + ".vtt";
	string out = vp.parent_path().string() + "/" + fn;
	cmd = str(format(cmd) % video % sid % out);
	LOGI("sub_stream_to_vtt cmd=%s", cmd.c_str());
	bp::child c(cmd, bp::std_err > is);
	c.wait();
    return c.exit_code();
}
std::string FFmpeg::get_media_streams(const std::string& media)
{
	bp::ipstream is; //reading pipe-stream
	string cmd = ff_ + " -hide_banner -i \"%1%\" ";
	cmd = str(format(cmd) % media);
	LOGI("cmd=%s", cmd.c_str());
    bp::child c(cmd, bp::std_err > is);
    std::vector<std::string> data;
    std::string line;
	std::regex r(R"(Stream\s#0:\d+)");
    std::smatch sm;
    while (c.running() && std::getline(is, line, '\n') && !line.empty())
	{
		if( regex_search(line, sm, r) )
		{
			// LOGI("matched line=%s", line.c_str());
			data.push_back(line);
		}		
		else
		{
			// LOGI("not matched line=%s", line.c_str());
		}
	}
    c.wait();
	auto output = Util::join(data, "\n");
	LOGI("get_media_streams output=%s", output.c_str());
	return output;
}
int FFmpeg::get_video_duration_secs(const std::string& video , int& sec)
{
	bp::ipstream is; //reading pipe-stream
	string cmd = ff_ + " -hide_banner -i \"%1%\"";
	cmd = str(format(cmd) % video);
    bp::child c(cmd, bp::std_err > is);
    std::string line;
	std::regex r(R"(Duration:\s([^,]+))");
    std::smatch sm;
    while (c.running() && std::getline(is, line) && !line.empty())
	{
		if( regex_search(line, sm, r) )
		{
			sec = Util::dur_as_sec(sm[1]);			
			// std::cout << sm[1] << " = " << s << " seconds" << '\n';
			break;
		}
	}
	c.wait();
    return c.exit_code();
}
int FFmpeg::get_video_codec(const std::string& video , std::string& codec)
{
	// codec=hevc(yuv420p10le) or h264(yuv420p) or mpeg4(yuv420p)
	bp::ipstream is; //reading pipe-stream
	string cmd = ff_ + " -hide_banner -i \"%1%\"";
	cmd = str(format(cmd) % video);
    bp::child c(cmd, bp::std_err > is);
    std::string line;
	std::regex r(R"(Video:\s(\w+).+?,\s(\w+))");
    std::smatch sm;
    while (c.running() && std::getline(is, line) && !line.empty())
	{
		if( regex_search(line, sm, r) )
		{
			codec = sm[1].str() + "(" + sm[2].str() + ")";			
			// std::cout << sm[1] << " , " << sm[2] << '\n';
			break;
		}
	}
	c.wait();
    return c.exit_code();
}
int FFmpeg::convert_to_x264(std::string video)
{
	// convert to h264+mp3 (not libfdk_aac)
	string full_path = AP::instance().full_store_path(video);
	Json noty;
	noty["cmd"] = "ffmpeg_progress";
	int total_sec{0};
	fs::path vp{full_path};
	get_video_duration_secs(full_path, total_sec);
	bp::ipstream is; //reading pipe-stream
	string cmd = ff_ +
	" -y -hide_banner -i \"%1%\" "
	"-map 0:v -map 0:a? -map 0:s? "
	"-c:v libx264 "
	"-pix_fmt yuv420p "
	"-c:a libmp3lame "
	"-c:s mov_text "
	"\"%2%\""
	;
	string fn = vp.stem().string() + "_x264.mp4";
	string out = vp.parent_path().string() + "/" + fn;
	cmd = str(format(cmd) % full_path % out);
	LOGI( cmd );
    bp::child c(cmd, bp::std_err > is);
    std::string line;
	std::regex r(R"(time=([^\s]+).+(speed=.+?x))");
    std::smatch sm;
    while (c.running() && std::getline(is, line, '\r') && !line.empty())
	{
		if( regex_search(line, sm, r) )
		{
			auto s = Util::dur_as_sec(sm[1]);
			float p = static_cast<float>(s)/total_sec * 100;
			noty["progress"] = str(format("%.2f%%") % p);
			noty["speed"] = sm[2];
			noty["video"] = video;
			noty["sid"] = "0:0";
			http_svr_->ws_to_all( noty.dump() );
			// LOGI("convert_to_x264: %s%%--%s", noty["progress"].get<string>().c_str(), noty["speed"].get<string>().c_str() );
			// converted: 0.620939%--speed=3.12x
		}
	}
    c.wait();
	http_svr_->ws_to_all( Util::refresh_files_noty() );
    return c.exit_code();
}
// void regex_test()
// {
// 	std::string log(R"(
//         Speed:	366
//         Mass:	35
//         Speed:	378
//         Mass:	32
//         Speed:	400
// 	Mass:	30)");
//     std::regex r(R"(Speed:\t(\d*))");
//     std::smatch sm;
//     while(regex_search(log, sm, r))
//     {
//         // std::cout << sm.str() << '\n';
// 		std::cout << sm[1] << '\n';
//         log = sm.suffix();
//     }
// }

#pragma once

#include "common.h"

class HttpSvr;
class FFmpeg
{
    std::string ff_;
    HttpSvr *http_svr_;
public:
    FFmpeg(const std::string& ff, HttpSvr *http_svr);
    int get_video_duration_secs(const std::string& video , int& sec);
    int get_video_codec(const std::string& video , std::string& codec);
    int convert_to_x264(std::string video);
    std::string get_media_streams(const std::string& media);
    int sub_stream_to_vtt(const std::string& video, std::string sid, std::string lang = "");
    int srt_to_vtt(const std::string& srt);
    int audio_stream_to_mp3(const std::string& video, std::string sid, std::string lang = "");
};

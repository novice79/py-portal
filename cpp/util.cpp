
#include <magic.h>
#include "util.h"

using namespace std;

namespace Util
{
uint32_t serial_no()
{
	static atomic<uint32_t> s_serial_no(0);
	return s_serial_no++;
}
std::string ep_to_string(boost::asio::ip::udp::endpoint ep)
{
	string s_ep = ep.address().to_string();
	s_ep += ":" + to_string(ep.port());
	return s_ep;
}
bool Base64Encode(const string& input, string& output) {
    typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<string::const_iterator, 6, 8> > Base64EncodeIterator;
    stringstream result;
    copy(Base64EncodeIterator(input.begin()) , Base64EncodeIterator(input.end()), ostream_iterator<char>(result));
    output = result.str();
    return output.empty() == false;
}

bool Base64Decode(const string& input, string& output) {
    typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<string::const_iterator>, 8, 6> Base64DecodeIterator;
    stringstream result;
    try {
        copy(Base64DecodeIterator(input.begin()) , Base64DecodeIterator(input.end()), ostream_iterator<char>(result));
    } catch(...) {
        return false;
    }
    output = result.str();
    return output.empty() == false;
}
std::vector<uint8_t> uuid()
{
	static auto rg = boost::uuids::random_generator();
	boost::uuids::uuid u = rg();
    std::vector<uint8_t> v(u.size());
    std::copy(u.begin(), u.end(), v.begin());
	// boost::lexical_cast<std::string>(uuid)
	// boost::uuids::to_string(uuid)
	return v;
}
vector<string> split(const string &s, std::string delim)
{
	vector<string> strs;
	boost::split(strs, s, boost::is_any_of(delim));
	return strs;
}
std::string join(const std::vector<std::string> &ss, std::string delim)
{
	std::string joined = boost::algorithm::join(ss, delim);
	return joined;
}
int dur_as_sec(const std::string &d)
{
	// d = "02:07:58.30"
	auto v = split(d, ":");
	std::reverse(v.begin(), v.end()); 
	// cout << "v[2]=" << v[2] << " ; " << "v[1]=" << v[1] << " ; " << "v[0]=" << v[0] << endl;
	int secs = std::stoi(v[2]) * 3600 + std::stoi(v[1]) * 60 + std::stoi(v[0]);
	return secs;
}
std::string byte2str(const uint8_t *bytes, int size)
{
	static char const hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	std::string str;
	for (int i = 0; i < size; ++i)
	{
		const char ch = bytes[i];
		str.append(&hex[(ch & 0xF0) >> 4], 1);
		str.append(&hex[ch & 0xF], 1);
	}
	return str;
}
std::string byte2str(const std::vector<uint8_t>& v)
{
	return byte2str( &v[0], v.size() );
}
std::string hexStr(const std::string &buff)
{
	constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
							   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	std::string s(buff.size() * 2, ' ');
	for (auto i = 0; i < buff.size(); ++i)
	{
		s[2 * i] = hexmap[(buff[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[buff[i] & 0x0F];
	}
	return s;
}
std::string file_type(const std::string &path)
{
	static magic_t handle = NULL;
	if (!handle)
	{
		// MAGIC_MIME: A shorthand for MAGIC_MIME_TYPE | MAGIC_MIME_ENCODING.
		handle = ::magic_open(MAGIC_NONE | MAGIC_MIME);
		if (handle == NULL)
		{
			LOGD("initialize magic library failed");
		}
		string magic_lib = AP::instance().prefab_path() + "/magic.mgc";
		if( ::magic_load( handle, magic_lib.c_str() ) != 0 )
		{
			LOGD("cannot load magic database -%s", magic_error(handle));
			magic_close(handle);
		}
		else
		{
			LOGD("load magic database succeed!");
		}
	}
	return ::magic_file(handle, path.c_str());
}
bool is_pac(const std::string &path)
{
	using boost::iequals;
    auto const ext = [&path]()->string
    {
        auto const pos = path.rfind(".");
        if(pos == string::npos)
            return "";
        return path.substr(pos);
    }();
	return iequals(ext, ".pac");
}
std::string to_json(std::map <std::string, std::string> data)
{
	Json json;
	for( auto const& [key, val] : data )
	{
		json[key] = val;
	}
	return json.dump();
}
std::string mime_type(const std::string &path)
{
    using boost::iequals;
    auto const ext = [&path]()->string
    {
        auto const pos = path.rfind(".");
        if(pos == string::npos)
            return "";
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    // if(iequals(ext, ".txt"))  return "text/plain; charset=utf-8";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
	if(iequals(ext, ".pac"))  return "application/x-ns-proxy-autoconfig";
	return file_type(path);
    // return "application/text";
}
std::string get_tid()
{
	std::thread::id this_id = std::this_thread::get_id();
	stringstream ss;
	ss << this_id;
	string tid = ss.str();
	return tid;
}
int uncompress(const std::string &zip, const std::string &to)
{
	using boost::format;
	auto ft = file_type(zip);
	string cmd;
	std::size_t found = ft.find("/zip");
	do
	{
		if (found != std::string::npos)
		{
			cmd = str(format("7z x -aoa -o%1% %2%") % to % zip );
			break;
		}
		found = ft.find("/x-7z");
		if (found != std::string::npos)
		{
			cmd = str(format("7z x -aoa -o%1% %2%") % to % zip );
			break;
		}
		found = ft.find("/x-rar");
		if (found != std::string::npos)
		{
			cmd = str(format("unrar x -o+ %1% %2%") % zip % to );
			break;
		}
	} while (0);
	if( !cmd.empty() )
	{
		// error: format string is not a string literal
		LOGD( "Util::uncompress: %s", cmd.c_str() );
		return bp::system(cmd);
	}
	return -1;
}
// prerequisite: this function is used for get files info in mystore
// in: relative path, out: relative path. so first need converted to full path then do the job
std::string get_files_json(const std::string &path)
{	
	fs::path p{AP::instance().full_store_path(path)};
	if( !fs::exists(p) ) return "{}";
	Json j;
	j["cmd"] = "get_file_list";
	j["files"] = Json::array();
	std::vector<fs::path> v;

	for (auto &&x : fs::directory_iterator(p))
	{
		if(x.path().string() == AP::instance().tmp())
			continue;
		v.push_back(x.path());
	}
	// leave client to sort
	// std::sort(v.begin(), v.end());
	for (auto &&x : v)
	{
		Json fi;
		string fn = x.filename().string();
		std::time_t t = fs::last_write_time(x.string());
		std::tm tm = *std::localtime(&t);
		std::ostringstream ss;
		ss << std::put_time(&tm, "%F %T");
		std::string time_str = ss.str();
		fi["name"] = fn;
		fi["time"] = time_str;
		fi["path"] = AP::instance().rel_store_path( x.string() );
		if( fs::is_directory(x) )
		{
			fi["type"] = "dir";
			j["files"].push_back(fi);
			continue;
		}
		auto type = file_type( x.string() );
		fi["type"] = (!type.empty() ? type : "UNKOWN");
		fi["size"] = fs::file_size(x);
		fi["ext"] = x.extension().string();
		// LOGI("fi=%1%", fi.dump());
		j["files"].push_back(fi);
	}
	// LOGI("j=%1%", j.dump());
	return j.dump();
}
std::string refresh_files_noty()
{
	Json j;
	j["cmd"] = "refresh_file_list";
	return j.dump();
}
std::string string_to_hex(const std::string &input)
{
	static const char *const lut = "0123456789ABCDEF";
	size_t len = input.length();

	std::string output;
	output.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = input[i];
		output.push_back(lut[c >> 4]);
		output.push_back(lut[c & 15]);
	}
	return output;
}
std::string random_string(size_t length)
{
	auto randchar = []() -> char {
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}
std::string decodeHex(const std::string &source)
{
	if (std::string::npos != source.find_first_not_of("0123456789ABCDEFabcdef"))
	{
		// you can throw exception here
		return {};
	}

	union {
		uint64_t binary;
		char byte[8];
	} value{};

	auto size = source.size(), offset = (size % 16);
	std::vector<uint8_t> binary{};
	binary.reserve((size + 1) / 2);

	if (offset)
	{
		value.binary = std::stoull(source.substr(0, offset), nullptr, 16);

		for (auto index = (offset + 1) / 2; index--;)
		{
			binary.emplace_back(value.byte[index]);
		}
	}

	for (; offset < size; offset += 16)
	{
		value.binary = std::stoull(source.substr(offset, 16), nullptr, 16);
		for (auto index = 8; index--;)
		{
			binary.emplace_back(value.byte[index]);
		}
	}

	return string((char *)&binary[0], binary.size());
}

std::string urlEncode(const std::string &toEncode)
{
	std::ostringstream out;

	for (std::string::size_type i = 0; i < toEncode.length(); ++i)
	{
		short t = toEncode.at(i);

		if (
			t == 45 ||				 // hyphen
			(t >= 48 && t <= 57) ||  // 0-9
			(t >= 65 && t <= 90) ||  // A-Z
			t == 95 ||				 // underscore
			(t >= 97 && t <= 122) || // a-z
			t == 126				 // tilde
		)
		{
			out << toEncode.at(i);
		}
		else
		{
			out << charToHex(toEncode.at(i));
		}
	}

	return out.str();
}

std::string urlDecode(const std::string &toDecode)
{
	std::ostringstream out;

	for (std::string::size_type i = 0; i < toDecode.length(); ++i)
	{
		if (toDecode.at(i) == '%')
		{
			std::string str(toDecode.substr(i + 1, 2));
			out << hexToChar(str);
			i += 2;
		}
		else
		{
			out << toDecode.at(i);
		}
	}

	return out.str();
}

std::string charToHex(unsigned char c)
{
	short i = c;
	std::stringstream s;
	s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;
	return s.str();
}

unsigned char hexToChar(const std::string &str)
{
	short c = 0;
	if (!str.empty())
	{
		std::istringstream in(str);
		in >> std::hex >> c;
		if (in.fail())
		{
			throw std::runtime_error("stream decode failure");
		}
	}
	return static_cast<unsigned char>(c);
}
} // namespace Util

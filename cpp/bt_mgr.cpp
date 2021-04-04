#include "bt_mgr.h"

const std::string BtMgr::creator_str = "novice-torrent";
std::string BtMgr::tf2mag(std::string fp)
{
	lt::error_code ec;
	lt::torrent_info torrent_info(fp, ec);

	return lt::make_magnet_uri(torrent_info);
}
std::string BtMgr::branch_path(std::string const &f)
{
	if (f.empty())
		return f;

#ifdef TORRENT_WINDOWS
	if (f == "\\\\")
		return "";
#endif
	if (f == "/")
		return "";

	auto len = f.size();
	// if the last character is / or \ ignore it
	if (f[len - 1] == '/' || f[len - 1] == '\\')
		--len;
	while (len > 0)
	{
		--len;
		if (f[len] == '/' || f[len] == '\\')
			break;
	}

	if (f[len] == '/' || f[len] == '\\')
		++len;
	return std::string(f.c_str(), len);
}

// do not include files and folders whose
// name starts with a .
bool BtMgr::file_filter(std::string const &f)
{
	if (f.empty())
		return false;

	char const *first = f.c_str();
	char const *sep = strrchr(first, '/');
	// if there is no parent path, just set 'sep'
	// to point to the filename.
	// if there is a parent path, skip the '/' character
	if (sep == nullptr)
		sep = first;
	else
		++sep;

	// return false if the first character of the filename is a .
	if (sep[0] == '.')
		return false;

	// std::cerr << f << "\n";
	return true;
}
int BtMgr::create_torrent(std::string fp, std::vector<std::string> trackers, std::string comment)
{
	if (!fs::exists(fp))
	{
		printf("%s not exist \n", fp.c_str());
		return -1;
	}
	int piece_size = 0;
	lt::create_flags_t flags = {};
	flags |= lt::create_torrent::v1_only;
	flags |= lt::create_torrent::modification_time;
	lt::file_storage fs;
	lt::add_files(fs, fp, file_filter, flags);
	if (fs.num_files() == 0)
	{
		std::cerr << "no files specified.\n";
		return 1;
	}
	lt::create_torrent t(fs, piece_size, flags);
	for (int i = 0; i < trackers.size(); ++i)
	{
		t.add_tracker(trackers[i], i);
	}
	auto const num = t.num_pieces();
	lt::set_piece_hashes(t, branch_path(fp), [num](lt::piece_index_t const p) {
		std::cerr << "\r" << p << "/" << num;
	});
	std::cout << "\nthread id=" << std::this_thread::get_id() << std::endl;
	t.set_creator(creator_str.c_str());
	t.set_comment(comment.c_str());
	std::vector<char> torrent;
	lt::bencode(back_inserter(torrent), t.generate());

	std::fstream out;
	out.exceptions(std::ifstream::failbit);
	auto torrent_file = fs::canonical(fp).replace_extension("torrent");
	out.open(torrent_file.string(), std::ios_base::out | std::ios_base::binary);
	out.write(torrent.data(), torrent.size());
	return 0;
}
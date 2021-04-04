#pragma once

#include "common.h"
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/create_torrent.hpp>

class BtMgr
{
    static std::string branch_path(std::string const& f);
    static bool file_filter(std::string const& f);
public:
    // torrent file to magnet_uri
    std::string tf2mag(std::string fp);
    // fp=full path
    int create_torrent(std::string fp, std::vector<std::string> trackers, std::string comment );
private:
    static const std::string creator_str;
};



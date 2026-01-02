#ifndef AV_LIBTORRENT_H_
#define AV_LIBTORRENT_H_

#include <string>

namespace av {
	namespace libtorrent {
		bool create_torrent(const std::string& in, const std::string& out_file);
	}
}

#endif

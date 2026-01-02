#ifndef AV_PATH_H_
#define AV_PATH_H_

#include "av_string.h"

namespace av {
	namespace path {
		std::tstring get_exe_name();
		std::tstring get_exe_dir();
		std::tstring append(const std::tstring& path, const std::tstring& app);
		bool create_dir(const std::tstring& path);
		bool exists(const std::tstring& path);
		bool dir_exists(const std::tstring& path);
		bool file_exists(const std::tstring& path);
		bool remove_dir_all(const std::tstring& path);
		bool remove_dir(const std::tstring& path);
		bool remove_file(const std::tstring& path);
		bool move_file(const std::tstring& source_path, const std::tstring& dest_path, bool force);
		bool move_file(const std::tstring& source_path, const std::tstring& dest_path);
		bool split(const std::tstring& path, std::tstring& p, std::tstring& f);
	}
}

#endif

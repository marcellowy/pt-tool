#include "av_base.h"
#include "av_path.h"

#include <filesystem>

#include "av_log.h"
#include "av_string.h"

namespace fs = std::filesystem;

#ifdef _WIN32
std::tstring av::path::get_exe_name() {
	wchar buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::tstring path(buffer);
	size_t pos = path.find_last_of(TEXT("\\/"));
	return (pos != std::tstring::npos) ? path.substr(pos + 1) : path;
}
#else
#include <unistd.h>
#include <limits.h>
std::tstring av::path::get_exe_name() {
	char buffer[PATH_MAX] = { 0 };
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len != -1) {
		buffer[len] = '\0';
		std::tstring path(buffer);
		size_t pos = path.find_last_of('/');
		return (pos != std::tstring::npos) ? path.substr(pos + 1) : path;
	}
	return "";
}
#endif

std::tstring av::path::append(const std::tstring& path, const std::tstring& app) {
	fs::path path_ = path;
	fs::path app_ = app;
	return av::str::toT((path_ / app_).string());
}

bool av::path::create_dir(const std::tstring& path) {
	if (dir_exists(path)) return true;
	fs::path path_ = path;
	try {
		// if exists
		return fs::create_directories(path_);
	}
	catch (const fs::filesystem_error& e) {
		loge("create_directories failed, path {}, err {}", av::str::toA(path), e.what());
		return false;
	}
}

bool av::path::exists(const std::tstring& path) {
	fs::path path_ = path;
	std::error_code ec;
	bool is_exists = fs::exists(path_, ec);
	if (ec) {
		loge("exists {} failed, err {}", av::str::toA(path), ec.message());
		return false;
	}
	return is_exists;
}

bool av::path::dir_exists(const std::tstring& path) {
	fs::path path_ = path;
	std::error_code ec;
	bool is_dir = fs::exists(path_, ec) && fs::is_directory(path_, ec);
	if (ec) {
		loge("dir_exists {} failed, err {}", av::str::toA(path), ec.message());
		return false;
	}
	return is_dir;
//#endif
}

bool av::path::file_exists(const std::tstring& path) {
	fs::path path_ = path;
	std::error_code ec;
	bool is_file = fs::exists(path_, ec) && fs::is_regular_file(path_, ec);
	if (ec) {
		loge("file_exists {} failed, err {}", av::str::toA(path), ec.message());
		return false;
	}
	return is_file;
}

bool av::path::remove_dir_all(const std::tstring& path) {
	if (!dir_exists(path)) return true;
	fs::path path_ = path;
	std::error_code ec;
	uintmax_t removed_count = fs::remove_all(path_, ec);
	if (ec) {
		loge("remove_dir_all {} failed, err {}", av::str::toA(path), ec.message());
		return false;
	}
	return true;
}


bool av::path::remove_dir(const std::tstring& path) {
	if (!dir_exists(path)) return true;
	fs::path path_ = path;
	std::error_code ec;
	bool ok = fs::remove(path_, ec);
	if (!ok || ec) {
		loge("remove_dir {} failed, err {}", av::str::toA(path), ec.message());
		return false;
	}
	return true;
}

bool av::path::remove_file(const std::tstring& path) {
	if (!file_exists(path)) {
		return true;
	}
	fs::path path_ = path;
	std::error_code ec;
	bool ok = fs::remove(path_, ec);
	if (!ok || ec) {
		loge("remove_file {} failed, err {}", av::str::toA(path), ec.message());
		return false;
	}
	return true;
}

bool av::path::move_file(const std::tstring& source_path, const std::tstring& dest_path, bool force) {
	fs::path source_path_ = source_path;
	fs::path dest_path_ = dest_path;
	std::error_code ec;
	if (!av::path::file_exists(source_path)) {
		loge("file {} not exists", av::str::toA(source_path));
		return false;
	}

	// check dst
	bool dest_exists = av::path::file_exists(dest_path);
	if (!force && dest_exists) {
		loge("dst {} exists", av::str::toA(dest_path));
		return false;
	}

	if (force && dest_exists) {
		std::error_code ec;
		fs::remove(dest_path, ec);
		if (ec) {
			loge("remove {} failed, err {}", av::str::toA(dest_path), ec.message());
			return false;
		}
	}

	// rename
	fs::rename(source_path, dest_path, ec);
	if (ec) {
		loge("rename {} to {} failed, err {}", av::str::toA(source_path), av::str::toA(dest_path), ec.message());
		return false;
	}
	return true;
}

bool av::path::move_file(const std::tstring& source_path, const std::tstring& dest_path) {
	return move_file(source_path, dest_path, false);
}

bool av::path::split(const std::tstring& path, std::tstring& p, std::tstring& f) {
	fs::path file_path(path);
	fs::path directory = file_path.parent_path();
	fs::path filename = file_path.filename();
	p = av::str::toT(directory.string());
	f = av::str::toT(filename.string());
	return true;
}
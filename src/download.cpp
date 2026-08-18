#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>

#include "av_worker.h"
#include "av_path.h"
#include "av_time.h"

#include "config.h"
#include "download.h"
#include "publish.h"

namespace fs = std::filesystem;
using namespace std::chrono;

Download::Download() {

}

Download::~Download() {
	stop();
}

int64_t Download::spaceFreeSize(const std::tstring& root) {

	// 使用 std::error_code 可以避免在查询失败时抛出异常
	std::error_code ec;
	fs::space_info si = fs::space(root, ec);

	if (ec) {
		std::cerr << "get disk space failed: " << ec.message() << std::endl;
		return false;
	}

	//
	constexpr auto G = 1024 * 1024 * 1024;
	logi("path: {}, total size: {} GB, free size: {} GB, available size (non-privileged): {} GB",
		av::str::toA(root), si.capacity / G, si.free / G, si.available / G);
	return si.free;
}

int64_t Download::getDirSize(const std::tstring& dir) {
	fs::path dir_path(dir);
	std::uintmax_t total_size = 0;

	// 检查路径是否存在且为目录
	if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
		throw std::runtime_error("path does not exist or is not a directory");
	}

	// 递归遍历目录
	try {
		for (const auto& entry : fs::recursive_directory_iterator(
			dir_path,
			fs::directory_options::skip_permission_denied)) {
			// 只累加普通文件的大小，忽略目录、符号链接等
			if (fs::is_regular_file(entry.status())) {
				total_size += fs::file_size(entry);
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		// 处理可能发生的其他错误（例如遍历过程中目录被删除）
		logw("error occurred during directory traversal: {}", e.what());
		return 0;
	}
	return total_size;
}

bool Download::task(const std::tstring& src_dir, const std::tstring& dst_dir) {
	// 复制目录
	auto copy_dir = [this](const fs::path& src_dir, const fs::path& dst_dir) -> bool {
		try {
			fs::copy(src_dir, dst_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
			logi("copy dir {} to {} success!", av::str::toA(src_dir), av::str::toA(dst_dir));
			return true;
		}
		catch (const std::exception& e) {
			logw("copy dir {} to {} failed! {}", av::str::toA(src_dir), av::str::toA(dst_dir), e.what());
			return false;
		}
		};

	// 复制文件
	auto copy_file = [this](const fs::path& src_file, const fs::path& dst_file) -> bool {
		try {
			fs::copy_file(src_file, dst_file, fs::copy_options::overwrite_existing);
			logi("copy file {} to {} success!", av::str::toA(src_file), av::str::toA(dst_file));
			return true;
		}
		catch (const std::exception& e) {
			logw("copy file {} to {} failed! {}", av::str::toA(src_file), av::str::toA(dst_file), e.what());
			return false;
		}
		};

	// 检查文件是否可复制(下载)
	auto is_file_downloadable = [this](const fs::path& file) -> bool {
		// 检查文件是否存在
		if (!std::filesystem::exists(file)) {
			logw("file {} not exists!", av::str::toA(file));
			return false;
		}
		// 检查文件大小是否超过当前磁盘剩余空间
		int64_t file_size = std::filesystem::file_size(file);
		int64_t free_size = spaceFreeSize(file.parent_path()) + 1024 * 1024 * 1024 * 10;
		if (free_size < file_size) {
			logw("file size {} > free size {}, not enough space!", file_size, free_size);
			return false;
		}
		return true;
		};

	// 检查目录里的内容是否可复制(下载)
	auto is_dir_downloadable = [this, &src_dir, &dst_dir](const fs::path& dir) -> bool {
		// 检查目录是否存在
		if (!std::filesystem::exists(dir)) {
			logw("dir {} not exists!", av::str::toA(dir));
			return false;
		}
		// 检查目录是否为空
		if (std::filesystem::is_empty(dir)) {
			logw("dir {} is empty!", av::str::toA(dir));
			return false;
		}

		// 检查是否存在固定格式文件
		{
#ifdef _UNICODE
			auto json_file = av::path::append(dir.wstring(), TEXT("media_info.json"));
#else
			auto json_file = av::path::append(dir.string(), TEXT("media_info.json"));
#endif // _UNICODE
			std::ifstream json_ifs(json_file, std::ios::in | std::ios::binary);
			if (!json_ifs.is_open()) {
				logw("file {} not exists! {}:{}",
					av::str::toA(json_file), std::to_string(errno), std::strerror(errno));
				return false;
			}
			json_ifs.seekg(0, std::ios::end);
			std::streamsize size = json_ifs.tellg();
			json_ifs.seekg(0, std::ios::beg);
			std::string content(size, '\0'); // 预分配空间
			if (!json_ifs.read(content.data(), size)) {
				logw("read file {} failed! {}:{}",
					av::str::toA(json_file), std::to_string(errno), std::strerror(errno));
				return false;
			}

			// 解析JSON内容
			nlohmann::json j;
			AVSInfo avs_info;
			try {
				j = nlohmann::json::parse(content);
				j.get_to(avs_info);
			}
			catch (const nlohmann::json::parse_error& e) {
				logw("json::parse_error {}", e.what());
				return false;
			}
			catch (const std::exception& e) {
				logw("std::exception {}", e.what());
				return false;
			}

			// 检查json内容 
			if (avs_info.created_time.empty()) {
				logw("json created_time is empty!");
				return false;
			}

			// created_time 是 UTC+8 时间；diff_now 使用固定的 UTC+8 时区计算，
			// 不依赖运行机器的本地时区。
			int64_t diff_time_seconds = 0;
			try {
				if (!av::time::diff_now(av::str::toA(avs_info.created_time), diff_time_seconds)) {
					logw("diff time failed, time: {}", av::str::toA(avs_info.created_time));
					return false;
				}
			}
			catch (const std::exception& e) {
				logw("parse created_time failed, time: {}, error: {}",
					av::str::toA(avs_info.created_time), e.what());
				return false;
			}

			if (diff_time_seconds < duration_cast<seconds>(std::chrono::hours(24)).count()) {
				logw("time not reached, time: {}, elapsed seconds: {}",
					av::str::toA(avs_info.created_time), diff_time_seconds);
				return false;
			}
		}

		// 检查目录大小是否超过当前磁盘剩余空间
		{
#ifdef _UNICODE
			int64_t dir_size = getDirSize(av::str::toT(dir.wstring()));
#else
			int64_t dir_size = getDirSize(av::str::toT(dir.string()));
#endif // _UNICODE
			int64_t free_size = spaceFreeSize(dst_dir) + 1024 * 1024 * 1024 * 10;

			//
			if (free_size < dir_size) {
				logw("dir size {} > free size {}, not enough space!", dir_size, free_size);
				return false;
			}
		}
		return true;
		};

	//
	std::error_code ec;
	for (const auto& entry : std::filesystem::directory_iterator(src_dir, ec)) {
		if (entry.is_regular_file()) {
			// 
			logi("file {}", av::str::toA(entry.path()));
			if (!is_file_downloadable(entry.path())) {
				// 不可复制状态
				continue;
			}
#ifdef _UNICODE
			auto filename = entry.path().filename().wstring();
			auto ext = entry.path().extension().wstring();
#else
			auto filename = entry.path().filename().string();
			auto ext = entry.path().extension().string();
#endif // _UNICODE
			logi("file name: {}, ext: {}", av::str::toA(filename), av::str::toA(ext));
			auto dst_file = av::path::append(dst_dir, filename);
			if (!copy_file(entry.path(), dst_file)) {
				av::path::remove_file(dst_file);	// 如果复制失败, 删除目标文件
				logw("copy file {} to {} failed!", av::str::toA(entry.path()), av::str::toA(dst_dir));
				continue;
			}
			// 如果复制成功, 则删除源文件
#ifdef _UNICODE
			av::path::remove_file(entry.path().wstring());
#else
			av::path::remove_file(entry.path().string());
#endif
		}
		else if (entry.is_directory()) {
			logi("dir {}", av::str::toA(entry.path()));
			if (!is_dir_downloadable(entry.path())) {
				// 不可复制状态
				continue;
			}
			fs::path p(entry.path());
			if (entry.path().filename() == TEXT(".") || entry.path().filename() == TEXT("")) {
				p = p.parent_path();
			}
			if (p.has_extension()) {
				p = p.parent_path();
			}
#ifdef _UNICODE
			auto lastp = p.filename().wstring();
			auto entry_path = entry.path().wstring();
#else
			auto lastp = p.filename().string();
			auto entry_path = entry.path().string();
#endif // _UNICODE

			// 复制目录
			auto dst_dir_val = av::path::append(dst_dir, lastp);
			if (!copy_dir(entry.path(), dst_dir_val)) {
				logw("copy dir {} to {} failed!", av::str::toA(entry.path()), av::str::toA(dst_dir_val));
				av::path::remove_dir_all(dst_dir_val);	// 如果复制失败, 删除目标目录
				continue;
			}

			// 复制成功删除源目录
			if (!av::path::remove_dir_all(entry_path)) {
				logw("remove dir {} failed!", av::str::toA(entry_path));
			}
		}
		else {
			// 不处理其他类型的文件
			logi("skip {}", av::str::toA(entry.path()));
		}
	}
	if (ec) {
		logw("list source dir {} failed! {}", av::str::toA(dst_dir), ec.message());
		return false;
	}

	return true;
}

bool Download::start() {
	auto& config = Config::instance();
	for (auto& cycle : config.mteam.download_cycle) {
		auto c = std::make_shared<av::cron::Cron>(cycle.name, cycle.pattern, [this] {
			logi("do download task");

			//
			auto& config = Config::instance();
			if (!std::filesystem::exists(config.mteam.source_dir)) {
				logw("source dir {} not exists!", av::str::toA(config.mteam.source_dir));
				return false;
			}
			if (!std::filesystem::exists(config.mteam.seed_dir))
			{
				logw("dst dir {} not exists!", av::str::toA(config.mteam.seed_dir));
				return false;
			}

			//	
			av::worker::global_worker().sync([this, &config] {
				task(config.mteam.source_dir, config.mteam.seed_dir);
				}, 86400 * 1000);    // 24 hours
			return true;
			});

		if (!c->start()) {
			logw("add cron {} failed!", av::str::toA(cycle.name));
			return false;
		}
		m_cron.push_back(c);
	}
	return true;
}

bool Download::stop() {
	for (auto& c : m_cron) {
		c->stop();
	}
	m_cron.clear();
	return true;
}

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

using namespace std::chrono;

Download::Download() {

}

Download::~Download() {
	stop();
}

int64_t Download::spaceFreeSize(const fs::path& root) {

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

int64_t Download::getDirSize(const fs::path& dir) {
	std::uintmax_t total_size = 0;

	// 检查路径是否存在且为目录
	if (!fs::exists(dir) || !fs::is_directory(dir)) {
		throw std::runtime_error("path does not exist or is not a directory");
	}

	// 递归遍历目录
	try {
		for (const auto& entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied)) {
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

bool Download::copyDir(const fs::path& src_dir, const fs::path& dst_dir) {
	try {
		fs::copy(src_dir, dst_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		logi("copy dir {} to {} success!", av::str::toA(src_dir), av::str::toA(dst_dir));
		return true;
	}
	catch (const std::exception& e) {
		logw("copy dir {} to {} failed! {}", av::str::toA(src_dir), av::str::toA(dst_dir), e.what());
		return false;
	}
	return false;
}

bool Download::copyFile(const fs::path& src_file, const fs::path& dst_file) {
	try {
		fs::copy_file(src_file, dst_file, fs::copy_options::overwrite_existing);
		logi("copy file {} to {} success!", av::str::toA(src_file), av::str::toA(dst_file));
		return true;
	}
	catch (const std::exception& e) {
		logw("copy file {} to {} failed! {}", av::str::toA(src_file), av::str::toA(dst_file), e.what());
		return false;
	}
	return false;
}

bool Download::isDownloadable(const fs::path& src, const fs::path& dst_dir) {
	if (fs::is_regular_file(src)) {
		// 检查文件是否存在
		if (!fs::exists(src)) {
			logw("file {} not exists!", av::str::toA(src));
			return false;
		}

		// 检查文件大小是否超过当前磁盘剩余空间
		int64_t file_size = fs::file_size(src) + 1024LL * 1024 * 1024 * 10;	// 留出10GB的缓冲空间
		int64_t free_size = spaceFreeSize(dst_dir);
		if (free_size < file_size) {
			logw("file size {} > free size {}, not enough space!", file_size, free_size);
			return false;
		}
		return true;
	}

	// 检查目录是否存在
	if (!fs::exists(src)) {
		logw("dir {} not exists!", av::str::toA(src));
		return false;
	}
	// 检查目录是否为空
	if (fs::is_empty(src)) {
		logw("dir {} is empty!", av::str::toA(src));
		return false;
	}

	// 检查是否存在固定格式文件
	{
		auto tmp_p = src;
		auto json_file = tmp_p / TEXT("media_info.json");
#ifdef _UNICODE
		auto json_file_str = json_file.wstring();
#else
		auto json_file_str = json_file.string();
#endif // _UNICODE

		//
		std::ifstream json_ifs(json_file, std::ios::in | std::ios::binary);
		if (!json_ifs.is_open()) {
			logw("file {} not exists! {}:{}", av::str::toA(json_file_str), std::to_string(errno), std::strerror(errno));
			return false;
		}
		json_ifs.seekg(0, std::ios::end);
		std::streamsize size = json_ifs.tellg();
		json_ifs.seekg(0, std::ios::beg);
		std::string content(size, '\0'); // 预分配空间
		if (!json_ifs.read(content.data(), size)) {
			logw("read file {} failed! {}:{}", av::str::toA(json_file_str), std::to_string(errno), std::strerror(errno));
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
		int64_t dir_size = getDirSize(src) + 1024LL * 1024 * 1024 * 10;	// 留出10GB的缓冲空间
		int64_t free_size = spaceFreeSize(dst_dir);

		//
		if (free_size < dir_size) {
			logw("dir size {} > free size {}, not enough space!", dir_size, free_size);
			return false;
		}
	}
	return true;
}

bool Download::atomicCopyDir(const fs::path& src_dir, const fs::path& dst_dir) {
	logi("atomic copy dir {} to {}", av::str::toUtf8(src_dir), av::str::toUtf8(dst_dir));

	// 获取源目录的最后一级目录名
	auto last_dir_name = getLastDirName(src_dir);
	auto new_tmp_dst_dir = dst_dir / (TEXT(".") + last_dir_name);
	auto new_dst_dir = dst_dir / last_dir_name;

	//
	std::error_code ec;

	// 创建目标目录
	if (fs::exists(new_tmp_dst_dir)) {

		// 删除临时目录
		fs::remove_all(new_tmp_dst_dir, ec);
		if (ec) {
			logw("remove dir {} failed! {}", av::str::toUtf8(new_tmp_dst_dir), ec.message());
			return false;
		}
	}

	if (!fs::create_directories(new_tmp_dst_dir, ec)) {
		logw("create dir {} failed! {}", av::str::toUtf8(new_tmp_dst_dir), ec.message());
		return false;
	}

	// 复制
	if (!copyDir(src_dir, new_tmp_dst_dir)) {
		logw("copy dir {} to {} failed!", av::str::toUtf8(src_dir), av::str::toUtf8(new_tmp_dst_dir));
		return false;
	}

	// 如果目标目录已经存在，先删除它
	if(fs::exists(new_dst_dir)) {
		if (!fs::remove_all(new_dst_dir, ec)) {
			logw("remove dir {} failed! {}", av::str::toUtf8(new_dst_dir), ec.message());
			return false;
		}
	}

	// 重命名临时目录为真实目录
	fs::rename(new_tmp_dst_dir, new_dst_dir, ec);
	if (ec) {
		logw("rename dir {} to {} failed! {}", 
			av::str::toUtf8(new_tmp_dst_dir), av::str::toUtf8(new_dst_dir), ec.message());
		return false;
	}

	//
	return true;
}

bool Download::atomicCopyFile(const fs::path& src_file, const fs::path& dst_dir) {
	logi("atomic copy file {} to {}", av::str::toUtf8(src_file), av::str::toUtf8(dst_dir));

	// 
	auto tmp_dst_file = [&src_file, &dst_dir]() {

		// 以 . 开头的临时文件名
		fs::path new_filename = TEXT(".");
		new_filename += src_file.filename();

		// 目标临时文件路径
		auto new_tmp_dst_file = dst_dir;
		new_tmp_dst_file /= new_filename;

		//
		return new_tmp_dst_file;
		}();
	// 
	auto dst_file = [&src_file, &dst_dir]() {

		// 真实文件名
		fs::path new_filename = src_file.filename();

		// 目标文件路径
		auto new_dst_file = dst_dir;
		new_dst_file /= new_filename;

		//
		return new_dst_file;
		}();

	// 先复制到临时文件
	if (!copyFile(src_file, tmp_dst_file)) {
		logw("copy file {} to {} failed!", av::str::toUtf8(src_file), av::str::toUtf8(tmp_dst_file));
		return false;
	}

	// 再重命名为真实文件
	try {
		// 如果目标文件已经存在，先删除它
		if (fs::exists(dst_file)) {
			if (!fs::remove(dst_file)) {
				logw("remove file {} failed!", av::str::toUtf8(dst_file));
				return false;
			}
		}

		//
		fs::rename(tmp_dst_file, dst_file);
		logi("rename file {} to {} success!", av::str::toUtf8(tmp_dst_file), av::str::toUtf8(dst_file));
	}
	catch (const std::exception& e) {
		logw("rename file {} to {} failed! {}", av::str::toUtf8(tmp_dst_file), av::str::toUtf8(dst_file), e.what());
		return false;
	}

	return true;
}

std::tstring Download::getLastDirName(const fs::path& dir) {
	// 处理根路径
	if (dir == dir.root_path()) {
		return TEXT("");
	}

	fs::path name = dir.filename();
	// 如果 filename 为空（如路径以分隔符结尾），取父目录的 filename
	if (name.empty()) {
		name = dir.parent_path().filename();
	}

#ifdef _UNICODE
	return name.wstring();
#else
	return name.string();
#endif // _UNICODE

}

bool Download::task(const std::tstring& src_dir, const std::tstring& dst_dir) {

	//
	auto& config = Config::instance();
	if (!config.download.enable) return true;

	//
	const fs::path src_dir_path(src_dir);
	const fs::path dst_dir_path(dst_dir);

	//
	std::error_code ec;
	int count_of_downloads = 0;	// 下载数量计数器
	for (const auto& entry : fs::directory_iterator(src_dir, ec)) {
		if (count_of_downloads >= config.download.numberOfDownloadPerTime) {
			break;
		}

		//
		if (entry.is_regular_file()) {
			// 处理一般文件
			logi("file {}", av::str::toUtf8(entry.path()));

			//
			if (!isDownloadable(entry.path(), dst_dir)) {
				logw("file {} is not downloadable!", av::str::toUtf8(entry.path()));
				continue;
			}

			//
			if (!atomicCopyFile(entry.path(), dst_dir_path)) {
				logw("atomic copy file {} to {} failed!", av::str::toUtf8(entry.path()), av::str::toUtf8(dst_dir_path));
				continue;
			}
			logi("atomic copy file {} to {} success!", av::str::toUtf8(entry.path()), av::str::toUtf8(dst_dir_path));
			count_of_downloads++;
		}
		else if (entry.is_directory()) {
			// 处理目录
			logi("dir {}", av::str::toA(entry.path()));

			//
			if (!isDownloadable(entry.path(), dst_dir)) {
				logw("file {} is not downloadable!", av::str::toUtf8(entry.path()));
				continue;
			}

			//
			if (!atomicCopyDir(entry.path(), dst_dir_path)) {
				logw("atomic copy dir {} to {} failed!", av::str::toUtf8(entry.path()), av::str::toUtf8(dst_dir_path));
				continue;
			}
			logi("atomic copy dir {} to {} success!", av::str::toUtf8(entry.path()), av::str::toUtf8(dst_dir_path));
			count_of_downloads++;
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
			if (!fs::exists(config.download.source_dir)) {
				logw("source dir {} not exists!", av::str::toA(config.download.source_dir));
				return false;
			}
			if (!fs::exists(config.mteam.seed_dir))
			{
				logw("dst dir {} not exists!", av::str::toA(config.mteam.seed_dir));
				return false;
			}

			//
			task(config.download.source_dir, config.mteam.seed_dir);
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

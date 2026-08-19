// 从 115 下载文件到本地的线程
// 以前使用rsync总是有漏掉文件的情况
// 文件还没有下载完成就会被删除，导致文件丢失
#include <filesystem>
#include "av_cron.h"

namespace fs = std::filesystem;

class Download {
public:
	Download();
	~Download();
	bool task(const std::tstring& src_dir, const std::tstring& dst_dir);
	bool start();
	bool stop();
private:
	int64_t spaceFreeSize(const std::filesystem::path& root);
	int64_t getDirSize(const std::filesystem::path& dir);

	// 原子复制
	bool atomicCopyDir(const fs::path& src_dir, const fs::path& dst_dir);
	bool atomicCopyFile(const fs::path& src_file, const fs::path& dst_dir);

	// 复制
	bool copyDir(const fs::path& src_dir, const fs::path& dst_dir);
	bool copyFile(const fs::path& src_file, const fs::path& dst_file);

	// 是否可下载 
	// @param src: 文件或目录路径
	// @param dst_dir: 目标目录路径
	bool isDownloadable(const fs::path& src, const fs::path& dst_dir);

	// 获取目录最后一级,如果是顶层返回空
	std::tstring getLastDirName(const fs::path& dir);
private:
	std::vector<std::shared_ptr<av::cron::Cron>> m_cron;
};

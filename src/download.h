// 从 115 下载文件到本地的线程
// 以前使用rsync总是有漏掉文件的情况
// 文件还没有下载完成就会被删除，导致文件丢失

#include "av_cron.h"

class Download {
public:
	Download();
	~Download();
	bool task(const std::tstring& src_dir, const std::tstring& dst_dir);
	bool start();
	bool stop();
private:
	int64_t spaceFreeSize(const std::tstring& root);
	int64_t getDirSize(const std::tstring& dir);
private:
	std::vector<std::shared_ptr<av::cron::Cron>> m_cron;
};

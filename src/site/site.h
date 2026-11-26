#ifndef SITE_H_
#define SITE_H_

#include "av_media_info.h"
#include "av_string.h"
#include "av_pt_gen.h"
#include "av_pt_gen_mteam.h"

class PublishName {
public:
	std::tstring torrent_dir;
	std::tstring title;
	std::tstring video_filename;
	std::tstring torrent_filename;
};

class Site {
public:

	// 发布
	virtual bool publish(const av::media::Source& source) = 0;

	// 返回文件或目录名称
	virtual bool name(const av::media::Source& source, PublishName& publish_name) = 0;

	// 获取豆瓣信息
	virtual bool douban(const std::string& douban_id, av::ptgen::Douban& douban_info) = 0;
	// 虚析构函数
	virtual ~Site() = default;
};

#endif // !SITE_H_

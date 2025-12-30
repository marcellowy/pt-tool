#ifndef TEST_DEFINED_H_
#define TEST_DEFINED_H_

#include <string>
#include <vector>
#include "mteam/category.h"
#include "av_string.h"

static std::tstring publishPrefixSport		= TEXT("0@");
static std::tstring publishPrefixVariety	= TEXT("00@");
static std::tstring publishPrefixDiscover	= TEXT("000@");
static std::tstring publishPrefixCustom		= TEXT("0000@"); // 手动修正过的

enum class PublishObjType {
	Unknown = 0,	
	Dir,			// 目录
	File,			// 文件
};

// 这个type是从源文件解析出来的type,不是
enum class PublishSiteType {
	Unknown = 0,
	Movie,			// 电影
	Sport,			// 体育
	TVSeries,		// 电视剧
	Discover,		// 纪录片
	Variety,		// 综艺节目
};

struct PublishObj {

	// dirinfo
	std::tstring dir;
	std::tstring name;
	std::tstring fullpath;
	PublishObjType type;
	PublishSiteType type_site;

	// baseinfo
	std::tstring year;
	std::tstring name_chs;
	std::tstring sub_title;
	std::tstring title_prefix;
	std::tstring douban_id;
	mteam::category::Id category_id;
	std::tstring season;
	std::tstring name_eng;

	// video screenshot
	std::vector<std::tstring> screenshot_local;
	std::vector<std::tstring> screenshot_url;

	// mediainfo
	std::tstring mediainfo_text;
	std::tstring mediainfo_json;
};

#endif

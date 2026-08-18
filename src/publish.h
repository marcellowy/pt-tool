
#ifndef TEST_PUBLISH_H_
#define TEST_PUBLISH_H_

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <filesystem>

#include "defined.h"
#include "av_media_info.h"
#include "av_cron.h"
#include "site.h"

namespace fs = std::filesystem;

using json = nlohmann::json;
using namespace av::media;

// AVS
class AVSInfo {
public:
    int64_t width;
    int64_t height;
    std::tstring video_format;
    std::tstring scan_type;
    std::tstring audio_format;
    std::tstring title;
    std::tstring channel;
    std::tstring year;
    int64_t category;
    std::tstring created_time;

    friend void from_json(const json& j, AVSInfo& avs) {
        if (j.contains("width") && j["width"].is_number_integer()) {
            avs.width = j["width"].get<int64_t>();
        }
        if (j.contains("height") && j["height"].is_number_integer()) {
            avs.height = j["height"].get<int64_t>();
        }
        if (j.contains("videoFormat") && j["videoFormat"].is_string()) {
            avs.video_format = av::str::toT(j["videoFormat"].get<std::string>());
        }
        if (j.contains("scanType") && j["scanType"].is_string()) {
            avs.scan_type = av::str::toT(j["scanType"].get<std::string>());
        }
        if (j.contains("audioFormat") && j["audioFormat"].is_string()) {
            avs.audio_format = av::str::toT(j["audioFormat"].get<std::string>());
        }
        if (j.contains("title") && j["title"].is_string()) {
            avs.title = av::str::toT(j["title"].get<std::string>());
        }
        if (j.contains("channel") && j["channel"].is_string()) {
            avs.channel = av::str::toT(j["channel"].get<std::string>());
        }
        if (j.contains("year") && j["year"].is_string()) {
            avs.year = av::str::toT(j["year"].get<std::string>());
        }
        if (j.contains("category") && j["category"].is_number_integer()) {
            avs.category = j["category"].get<int64_t>();
        }
        if (j.contains("createdTime") && j["createdTime"].is_string()) {
            avs.created_time = av::str::toT(j["createdTime"].get<std::string>());
        }
    }
};

class Publish
{
public:
	Publish();
	Publish(std::shared_ptr<Site>& site, const std::tstring& dir);
	~Publish();
	void task();
	bool start();
	bool stop();
protected:
	std::vector<Source> readDir();
	bool getSiteType(Source& obj);
	
	int processDir(Source& obj);
	bool processFile(Source& obj);

	// map tv name
	void tvname(Source& obj);

	// 
	void setResolution(int64_t width, int64_t height, const av::media::ScanType& scan_type, Source& obj);

	// 
	void capitalizeWords(std::tstring& s);

private:
	std::tstring m_dir;
	std::shared_ptr<Site> m_site;
	std::vector<std::shared_ptr<av::cron::Cron>> m_cron;
};



#endif

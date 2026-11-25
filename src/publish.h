
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
#include "error_code.h"
#include "av_utils.h"

namespace fs = std::filesystem;

using json = nlohmann::json;
using namespace av::media;

//
class MediaInfo {
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
	std::string douban_id;

    friend void from_json(const json& j, MediaInfo& media_info) {
        if (j.contains("width") && j["width"].is_number_integer()) {
            media_info.width = j["width"].get<int64_t>();
        }
        if (j.contains("height") && j["height"].is_number_integer()) {
            media_info.height = j["height"].get<int64_t>();
        }
        if (j.contains("videoFormat") && j["videoFormat"].is_string()) {
            media_info.video_format = av::str::toT(j["videoFormat"].get<std::string>());
        }
        if (j.contains("scanType") && j["scanType"].is_string()) {
            media_info.scan_type = av::str::toT(j["scanType"].get<std::string>());
        }
        if (j.contains("audioFormat") && j["audioFormat"].is_string()) {
            media_info.audio_format = av::str::toT(j["audioFormat"].get<std::string>());
        }
        if (j.contains("title") && j["title"].is_string()) {
            media_info.title = av::str::toT(j["title"].get<std::string>());
        }
        if (j.contains("channel") && j["channel"].is_string()) {
            media_info.channel = av::str::toT(j["channel"].get<std::string>());
        }
        if (j.contains("year") && j["year"].is_string()) {
            media_info.year = av::str::toT(j["year"].get<std::string>());
        }
        if (j.contains("category") && j["category"].is_number_integer()) {
            media_info.category = j["category"].get<int64_t>();
        }
        if (j.contains("createdTime") && j["createdTime"].is_string()) {
            media_info.created_time = av::str::toT(j["createdTime"].get<std::string>());
        }
        if (j.contains("doubanId") && j["doubanId"].is_string()) {
            media_info.douban_id = j["doubanId"].get<std::string>();
        }
    }
};

class Publish
{
public:
	Publish();
    Publish(const std::tstring& dir);
	Publish(std::shared_ptr<Site>& site, const std::tstring& dir, const std::tstring& tgbot_token, const std::tstring& tgbot_chat_id);
	~Publish();
	void task();
	bool start();
	bool stop();

    std::vector<Source> readDir(PublishError& error);
    bool getSiteType(Source& source);
protected:
	int processDir(Source& source, 
        std::vector<std::pair<std::string, std::function<void()>>>& rollback_tasks,
        PublishError& error);
	bool processFile(Source& source, 
        std::vector<std::pair<std::string, std::function<void()>>>& rollback_tasks,
        PublishError& error);

	// map tv name
	void tvname(Source& source);

	// 
	void setResolution(int64_t width, int64_t height, const av::media::ScanType& scan_type, Source& source);

	// 
	void capitalizeWords(std::tstring& s);

    //
	void sendTGWarningMessage(const std::string& dir, PublishError error);

private:
	std::tstring m_dir;
	std::shared_ptr<Site> m_site;
	std::vector<std::shared_ptr<av::cron::Cron>> m_cron;

    //
    std::tstring m_tgbot_token;
    std::tstring m_tgbot_chat_id;

    //
    std::atomic_flag m_scoped_flag = ATOMIC_FLAG_INIT;
};



#endif

#include "publish.h"

#include <filesystem>

#include "av_log.h"
#include "av_string.h"
#include "av_path.h"
#include "av_async.h"
#include "av_time.h"
#include "av_mediainfo.h"
#include "av_media_info.h"

#include "config.h"
#include "parse_name.h"


namespace fs = std::filesystem;

Publish::Publish()
{
}

Publish::Publish(std::shared_ptr<Site>& site, const std::tstring& dir): m_site(std::move(site)), m_dir(dir) {

}

Publish::~Publish()
{
    stop();
}

bool Publish::start(){
    auto arr = readDir();
    for (auto& tmp : arr) {
        if (!processFile(tmp)) {
            logw("process file failed, dir {}, name {} get site type failed", av::str::toA(tmp.dir), av::str::toA(tmp.name));
            break;
        }
        m_site->publish(tmp);
        break;
    }
    return false;
}

bool Publish::stop() {
    return true;
}

bool Publish::getSiteType(Source& obj) {

    if (obj.type == SourceType::Dir) {
        return false;
    }

    std::tstring pre = TEXT("");

    // Sport
    pre = obj.name.substr(0,2);
    if (pre == publishPrefixSport) {
        obj.category = av::media::SourceCategory::Sport;
        //obj.category_id = mteam::category::Id::Sport;
        if (!parseSportName(obj)) {
            loge("publishSportName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }
        
    // Variety
    pre = obj.name.substr(0, 3);
    if (pre == publishPrefixVariety) {
        obj.category = av::media::SourceCategory::Variety;
        //obj.category_id = mteam::category::Id::TVSeries;
        if (!parseVarietyName(obj)) {
            loge("parseVarietyName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }

    // Discover
    pre = obj.name.substr(0, 4);
    if (pre == publishPrefixDiscover) {
        obj.category = av::media::SourceCategory::Discover;
        //obj.category_id = mteam::category::Id::Discover;
        if (!parseDiscoverName(obj)) {
            loge("parseDiscoverName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }

    // Custom
    pre = obj.name.substr(0, 5);
    if (pre == publishPrefixCustom) {
        if (!parseCustomName(obj)) {
            loge("parseCustomName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }

    // Movie
    if (obj.name.find(TEXT("€")) != std::tstring::npos) {
        obj.category = av::media::SourceCategory::Movie;
        //obj.category_id = mteam::category::Id::Movie;
        if (!parseMovieName(obj)) {
            loge("parseMovieName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }
    return false;
}

bool Publish::processDir(const std::tstring& path) { return false;  }

bool Publish::processFile(Source& obj) {
    logi("process {}, {}", av::str::toA(obj.dir), av::str::toA(obj.name));

    // site type
    if (!getSiteType(obj)) {
        logw("dir {}, name {} get site type failed", av::str::toA(obj.dir), av::str::toA(obj.name));
        return false;
    }

    // tv name
    tvname(obj);

    // mediainfo
    av::mediainfo::MediaInfo m(obj.fullpath);
    if (!m.parse()) {
        logw("get mediainfo failed, file {}", av::str::toA(obj.fullpath));
        return false;
    }

    logi("{} {}", av::str::toA(obj.fullpath), av::str::toA(obj.mediainfo_json));
    logi("{} {}", av::str::toA(obj.fullpath), av::str::toA(obj.mediainfo_text));

    // video resolution
    auto video_height = m.getVideo().height;
    auto video_width = m.getVideo().width;
    if (video_height == 4320) {
        obj.video_resolution = SourceVideoResolution::_8k;
    }
    else if (video_height == 2160 || (video_width == 3840 && video_height >= 1500 && video_height <= 2160)) {
        obj.video_resolution = SourceVideoResolution::_4k;
    }
    else if (video_height > 720 && video_height <= 1080) {
        auto scan_type = m.getVideo().scan_type;
        switch (scan_type) {
        case av::media::ScanType::Interlaced:
            obj.video_resolution = SourceVideoResolution::_1080i;
            break;
            obj.video_resolution = SourceVideoResolution::_1080p;
        case av::media::ScanType::MBAFF:
            break;
        case av::media::ScanType::Progressive:
            obj.video_resolution = SourceVideoResolution::_1080p;
            break;
        default:
            obj.video_resolution = SourceVideoResolution::Unknown;
            break;
        }
    }
    else if (video_height <= 720 && video_height > 480) {
        obj.video_resolution = SourceVideoResolution::_720;
    }
    else if (video_height <= 480) {
        obj.video_resolution = SourceVideoResolution::_480;
    }

    // video codec
    obj.video_codec = m.getVideo().codec;

    // audio codec
    obj.audio_codec = m.getAudio().codec;
   
    return true; 
}

std::vector<Source> Publish::readDir() {
    std::vector<Source> v;

    // check
    if (!av::path::dir_exists(m_dir)) {
        logw("{} not exists", av::str::toA(m_dir));
        return v;
    }

    // read
    for (const auto& entry : fs::directory_iterator(m_dir)) {
        Source obj;
        obj.name = av::str::toT(entry.path().filename().string());
        // 
        if (obj.name.find(TEXT("TPTV")) != std::tstring::npos) {
            // published
            continue;
        }

        //
        if (entry.is_regular_file()) {
            obj.type = SourceType::File;
        }
        else if (entry.is_directory()) {
            obj.type = SourceType::Dir;
        }
        obj.dir = m_dir;
        //logi("dir_ {}, obj.name {}, ", dir_, obj.name);
        try {
            obj.fullpath = av::path::append(m_dir, obj.name);
        }
        catch (const std::exception& e) {
            loge("exception {}", e.what());
        }

        v.emplace_back(obj);
    }
    return v;
}

void Publish::tvname(Source& obj) {
    if (obj.title_prefix.empty()) {
        return;
    }
    auto& tvs = Config::instance().tv_name;
    for (auto& tv : tvs) {
        if (tv.match == obj.title_prefix) {
            obj.title_prefix = tv.title_prefix;
            if (obj.category != av::media::SourceCategory::Movie) {
                obj.title_prefix = tv.sub_title_prefix + TEXT(" | ") + obj.sub_title;
            }
            return;
        }
    }
    obj.title_prefix = TEXT("");
}

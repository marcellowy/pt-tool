#include "publish.h"

#include "MediaInfoDLL/MediaInfoDLL.h"
//#include <MediaInfo/MediaInfo.h>

#include <filesystem>

#include "av_log.h"
#include "av_string.h"
#include "av_path.h"
#include "av_async.h"
#include "av_time.h"

#include "config.h"
#include "parse_name.h"

namespace fs = std::filesystem;

Publish::Publish()
{
}

Publish::Publish(const std::tstring& dir): dir_(dir) {

}

Publish::~Publish()
{
    stop();
}

bool Publish::start(){
    auto arr = readDir();
    for (auto& tmp : arr) {
        if (!getSiteType(tmp)) {
            logw("dir {}, name {} get site type failed", av::str::toA(tmp.dir), av::str::toA(tmp.name));
            continue;
        }
        if (!processFile(tmp)) {
            logw("process file failed, dir {}, name {} get site type failed", av::str::toA(tmp.dir), av::str::toA(tmp.name));
            break;
        }



        //break;
    }

    return false;
}

bool Publish::stop() {
    return true;
}

bool Publish::getSiteType(PublishObj& obj) {

    if (obj.type == PublishObjType::Dir) {

        return false;
    }

    std::tstring pre = TEXT("");

    // Sport
    pre = obj.name.substr(0,2);
    if (pre == publishPrefixSport) {
        obj.type_site = PublishSiteType::Sport;
        obj.category_id = mteam::category::Id::Sport;
        if (!parseSportName(obj)) {
            loge("publishSportName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }
        
    // Variety
    pre = obj.name.substr(0, 3);
    if (pre == publishPrefixVariety) {
        obj.type_site = PublishSiteType::Variety;
        obj.category_id = mteam::category::Id::TVSeries;
        if (!parseVarietyName(obj)) {
            loge("parseVarietyName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }

    // Discover
    pre = obj.name.substr(0, 4);
    if (pre == publishPrefixDiscover) {
        obj.type_site = PublishSiteType::Discover;
        obj.category_id = mteam::category::Id::Discover;
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

        if (obj.category_id == mteam::category::Id::TVSeries) {
            obj.type_site = PublishSiteType::TVSeries;
        } else if (obj.category_id == mteam::category::Id::Sport) {
            obj.type_site = PublishSiteType::Sport;
        } else if (obj.category_id == mteam::category::Id::Movie) {
            obj.type_site = PublishSiteType::Movie;
        } else if (obj.category_id == mteam::category::Id::Discover) {
            obj.type_site = PublishSiteType::Discover;
        } else {
            loge("unknown category_id {}", av::str::toA(mteam::category::to_string(obj.category_id)));
            return false;
        }
        return true;
    }

    // Movie
    if (obj.name.find(TEXT("€")) != std::tstring::npos) {
        obj.type_site = PublishSiteType::Movie;
        obj.category_id = mteam::category::Id::Movie;
        if (!parseMovieName(obj)) {
            loge("parseMovieName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
            return false;
        }
        return true;
    }
    return false;
}

bool Publish::processDir(const std::tstring& path) { return false;  }

bool Publish::processFile(PublishObj& obj) {
    logi("process {}, {}", av::str::toA(obj.dir), av::str::toA(obj.name));
    tvname(obj);
    /*if (!mediaInfo(obj, true)) {
        logw("get media info failed {}", av::str::toA(obj.fullpath));
        return false;
    }
    if (!gen(obj, false)) {
        logw("get media info failed {}", av::str::toA(obj.fullpath));
        return false;
    }*/
    logi("{}", av::str::toA(obj.mediainfo_json));
    logi("{}", av::str::toA(obj.mediainfo_text));
    return true; 
}

std::vector<PublishObj> Publish::readDir() {
    std::vector<PublishObj> v;

    // check
    if (!av::path::dir_exists(dir_)) {
        logw("{} not exists", av::str::toA(dir_));
        return v;
    }

    // read
    for (const auto& entry : fs::directory_iterator(dir_)) {
        PublishObj obj;
        obj.name = av::str::toT(entry.path().filename().string());
        // 
        if (obj.name.find(TEXT("TPTV")) != std::tstring::npos) {
            // published
            continue;
        }

        //
        if (entry.is_regular_file()) {
            obj.type = PublishObjType::File;
        }
        else if (entry.is_directory()) {
            obj.type = PublishObjType::Dir;
        }
        obj.dir = dir_;
        //logi("dir_ {}, obj.name {}, ", dir_, obj.name);
        try {
            obj.fullpath = av::path::append(dir_, obj.name);
        }
        catch (const std::exception& e) {
            loge("exception {}", e.what());
        }

        v.emplace_back(obj);
    }
    return v;
}

void Publish::tvname(PublishObj& obj) {
    if (obj.title_prefix.empty()) {
        return;
    }
    auto& tvs = Config::instance().tv_name;
    for (auto& tv : tvs) {
        if (tv.match == obj.title_prefix) {
            obj.title_prefix = tv.title_prefix;
            if (obj.type_site != PublishSiteType::Movie) {
                obj.title_prefix = tv.sub_title_prefix + TEXT(" | ") + obj.sub_title;
            }
            return;
        }
    }
    obj.title_prefix = TEXT("");
}

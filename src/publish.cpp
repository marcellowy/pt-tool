#include "publish.h"

#include <filesystem>

#include "av_log.h"
#include "av_string.h"
#include "av_path.h"
#include "av_async.h"
#include "av_time.h"
#include "av_mediainfo.h"
#include "av_media_info.h"
#include "av_pt_gen.h"
#include "av_ffmpeg.h"
#include "av_codec_stb_image_jpg.h"
#include "av_md5.h"

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
    auto& config = Config::instance();
    logi("process {}", av::str::toA(obj.fullpath));

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

    // mediainfo text
    obj.mediainfo_text = m.getText();

    // add douban info
    if (!obj.douban_id.empty()) {
        av::ptgen::Douban db;
        char buff[2048];
        snprintf(buff, sizeof(buff) - 1, av::str::toA(config.ptgen.url).c_str(), av::str::toA(obj.douban_id).c_str());
        std::string real_url(buff);
        if (!av::ptgen::get(av::str::toT(real_url), db)) {
            loge("get douban info failed!");
            return false;
        }

        // set douban info to obj
        obj.year = av::str::toT(db.year);
        obj.name_chs = av::str::toT(db.name_chs);
        obj.name_eng = av::str::toT(db.name_eng);
        obj.sub_title = av::str::toT(db.sub_title);
        obj.imdb_link = av::str::toT(db.imdb_link);
        obj.description = av::str::toT(db.description);
        obj.poster_img = av::str::toT( db.poster_img);
    }

    // add screenshot
    // 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960
    const std::vector<int64_t> capture_time = { 60, 120, 180, 240 };
    int capture_count = 0;
    av::codec::StbPNG stbPng([&obj, &capture_count](void* data, int size) {
        
        // screenshots dir
        std::tstring dir = av::path::get_exe_dir();
        dir = av::path::append(dir, TEXT("screenshots"));
        dir = av::path::append(dir, obj.fullpath_md5);
        if (!av::path::create_dir(dir)) {
            loge("create dir {} failed", av::str::toA(dir));
            return;
        }
        std::tstringstream oo;
        oo << TEXT("screenshot_") << capture_count << TEXT(".png");
        std::tstring filename = av::path::append(dir, oo.str());
        std::ofstream out_file(av::str::toA(filename), std::ios::binary);
        out_file.write(static_cast<char*>(data), size);  // 写入数据到文件

        obj.screenshot_local.push_back(filename);

        capture_count++;
    });
    if (!av::ffmpeg::captureFrame(obj.fullpath, capture_time, stbPng)) {
        loge("capture frame failed");
        return false;
    }

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
#ifdef _UNICODE
        obj.name = av::str::toT(entry.path().filename().wstring());
#else
        obj.name = av::str::toT(entry.path().filename().string());
#endif // _UNICODE

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
            std::string tmp_md5;
            av::hash::md5(av::str::toA(obj.fullpath), tmp_md5);
            obj.fullpath_md5 = av::str::toT(tmp_md5);
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

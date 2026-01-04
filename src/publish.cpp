#include "publish.h"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>

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
#include "av_translate.h"
#include "av_env.h"
#include "av_time.h"
#include "error_code.h"
#include "nlohmann/json.hpp"

#include "config.h"
#include "parse_name.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

// AVS
struct AVSInfo {
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
};

static void from_json(const json& j, AVSInfo& avs);

void from_json(const json& j, AVSInfo& avs) {
    if (j.contains("width") && j["width"].is_number_integer()) {
        avs.width = j["width"].get<int64_t>();
    }
    if (j.contains("height") && j["height"].is_number_integer()) {
        avs.height = j["height"].get<int64_t>();
    }
    if (j.contains("videoFormat") && j["videoFormat"].is_number_integer()) {
        avs.video_format = av::str::toT(j["videoFormat"].get<std::string>());
    }
    if (j.contains("scanType") && j["scanType"].is_number_integer()) {
        avs.scan_type = av::str::toT(j["scanType"].get<std::string>());
    }
    if (j.contains("audioFormat") && j["audioFormat"].is_number_integer()) {
        avs.audio_format = av::str::toT(j["audioFormat"].get<std::string>());
    }
    if (j.contains("title") && j["title"].is_number_integer()) {
        avs.title = av::str::toT(j["title"].get<std::string>());
    }
    if (j.contains("channel") && j["channel"].is_number_integer()) {
        avs.channel = av::str::toT(j["channel"].get<std::string>());
    }
    if (j.contains("year") && j["year"].is_number_integer()) {
        avs.year = av::str::toT(j["year"].get<std::string>());
    }
    if (j.contains("category") && j["category"].is_number_integer()) {
        avs.category = j["category"].get<int64_t>();
    }
    if (j.contains("createdTime") && j["createdTime"].is_number_integer()) {
        avs.created_time = av::str::toT(j["createdTime"].get<std::string>());
    }
}

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
        if (tmp.type == SourceType::File) {
            if (!processFile(tmp)) {
                logw("process file failed, dir {}, name {}", av::str::toA(tmp.dir), av::str::toA(tmp.name));
                continue;
            }
        }
        else if (tmp.type == SourceType::Dir) {
            auto ret = processDir(tmp);
            if (ret == ErrorCode::ErrTimeNotReached) {
                logw("{} time not reached", av::str::toA(tmp.name));
                continue;
            }
            logw("process dir failed, dir {}, name {}", av::str::toA(tmp.dir), av::str::toA(tmp.name));
            continue;
        }

        // preprocess succ
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

    av::async::Exit exit_filter_name([&obj] {
        av::str::replace_all(obj.name_chs, TEXT("(4k)"), TEXT("（4K）"));
    });

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

int Publish::processDir(Source& obj) {
    auto& config = Config::instance();
    logi("process {}", av::str::toA(obj.fullpath));

    // media_info.json, media_info.txt
    auto media_info_json_path = av::path::append(obj.fullpath, TEXT("media_info.json"));
    auto media_info_text_path = av::path::append(obj.fullpath, TEXT("media_info.txt"));
    if (!av::path::exists(media_info_json_path) || !av::path::exists(media_info_text_path)) {
        logw("{} or {} not exists", 
            av::str::toA(media_info_json_path), av::str::toA(media_info_text_path));
        return ErrorCode::ErrTimeNotReached;
    }

    // read file
    std::ifstream ifs;
    try {
        ifs.open(media_info_json_path, std::ios::in);
        if (!ifs.is_open()) {
            logw("open {} failed", av::str::toA(media_info_json_path));
            return -1;
        }
    }
    catch (const std::system_error& e) {
        logw("open {} failed, json::system_error {}", av::str::toA(media_info_json_path), e.what());
        return -1;
    }
    catch (const std::exception& e) {
        logw("open {} failed, std::exception {}", av::str::toA(media_info_json_path), e.what());
        return -1;
    }
    std::tstring json_content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    av::async::Exit exit_ifs([&ifs] {
        if (ifs.is_open()) ifs.close();
    });

    // parse json file
    AVSInfo avs_info;
    try {
        json _o;
        auto j = _o.parse(json_content);
        avs_info = j.get<AVSInfo>();
        av::media::ScanType scan_type;
        if (avs_info.scan_type == TEXT("Interlaced")) {
            scan_type = av::media::ScanType::Interlaced;
        }
        else if (avs_info.scan_type == TEXT("MBAFF")) {
            scan_type = av::media::ScanType::MBAFF;
        }
        else if (avs_info.scan_type == TEXT("Progressive")) {
            scan_type = av::media::ScanType::Progressive;
        }
        else {
            scan_type = av::media::ScanType::Unknown;
        }
        //
        setResolution(avs_info.width, avs_info.height, scan_type, obj);

        // 
        obj.video_codec = SourceVideoCodec::_avs;
        obj.audio_codec = SourceAudioCodec::_e_ac3_ddp;

        obj.title_prefix = avs_info.channel;
        obj.name_chs = avs_info.title;
        obj.sub_title = avs_info.title;
        obj.year = avs_info.year;

        // TODO: 相当于反向映射回来 这里的category要用外部公共的，
        // 但是提供的是一个m-team的category,这里硬编码一个转换
        //obj.category = avs_info.category;
    }
    catch (const json::parse_error& e) {
        logw("json::parse_error {}", e.what());
    }
    catch (const std::exception& e) {
        logw("std::exception {}", e.what());
    }

    // check time
    std::chrono::system_clock::duration duration;
    if (!av::time::diff_now(av::str::toA(avs_info.created_time), duration)) {
        logw("diff time failed, time: {}", av::str::toA(avs_info.created_time));
        return -1;
    }
    std::chrono::duration seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    if (seconds.count() < 24 * 3600) { // 24 hours
        logw("time not reached! wait time, {}", av::str::toA(obj.fullpath));
        return ErrorCode::ErrTimeNotReached;
    }

    // read dir
    int64_t ts_count = 0;
    std::vector<std::tstring> img_vec;
    for (const auto& entry : fs::directory_iterator(obj.fullpath)) {
#ifdef _UNICODE
        std::tstring name = entry.path().filename().wstring();
        std::tstring ext = entry.path().extension().wstring();
#else
        std::tstring name = entry.path().filename().string();
        std::tstring ext = entry.path().extension().string();
#endif // _UNICODE
        std::tstring fullpath = av::path::append(obj.fullpath, name);

        // process ts file
        if (ext == TEXT(".ts")) {
            ts_count++;
        }

        // process image file
        if (ext == TEXT(".jpg") || 
            ext == TEXT(".jpeg") || 
            ext == TEXT(".png")) {
            img_vec.push_back(fullpath);
        }
    }

    if (ts_count == 0) {
        // no ts
        return -1;
    }

    auto move_image = [&obj, &img_vec]() -> bool {
        // create dir
        std::tstring dir = av::path::get_exe_dir();
        dir = av::path::append(dir, TEXT("screenshots"));
        dir = av::path::append(dir, obj.fullpath_md5);
        if (!av::path::exists(dir)) {
            if (!av::path::create_dir(dir)) {
                loge("create dir {} failed", av::str::toA(dir));
                return false;
            }
        }

        // move
        for (auto& img: img_vec) {
            fs::path i = img;
#ifdef _UNICODE
            std::tstring tmp_name = i.filename().wstring();
#else
            std::tstring tmp_name = i.filename().string();
#endif // _UNICODE

            auto tmp = av::path::append(dir, tmp_name);
            if (!av::path::move_file(img, tmp, true)) {
                loge("move file {} to {} failed", img, tmp);
                return -1;
            }
            obj.screenshot_local.push_back(tmp);
        }
    };

    if (!move_image()) {
        logw("move image failed");
        return -1;
    }

    return ErrorCode::Success;
}

bool Publish::processFile(Source& obj) {
    auto& config = Config::instance();
    logi("process {}", av::str::toA(obj.fullpath));

    obj.source_id = av::media::from(config.mteam.source_id);
    obj.group_id = config.mteam.group_id;
    obj.seed_dir = config.mteam.seed_dir;

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
    setResolution(video_width, video_height, m.getVideo().scan_type, obj);

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

    // 首字母大写
    auto capitalizeWords = [](std::tstring& s) {
        bool newWord = true;
        for (tchar& c : s) {
            if (std::isspace(c)) {
                newWord = true;
            }
            else if (newWord) {
                c = std::toupper(static_cast<unsigned char>(c));
                newWord = false;
            }
        }
    };

    // add english name
    if (obj.name_eng.empty()) {
        av::translate::Translate t(config.rapidapi.key, config.rapidapi.host);
        if (!t.foo(obj.name_chs, obj.name_eng)) {
            loge("translate failed");
            return false;
        }
    }

    if (!empty(obj.name_eng)) {
        capitalizeWords(obj.name_eng);
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
#ifdef _UNICODE
            obj.file_suffix = entry.path().extension().wstring();
#else
            obj.file_suffix = entry.path().extension().string();
#endif
            av::str::replace(obj.file_suffix, TEXT("."), TEXT(""));
        }
        else if (entry.is_directory()) {
            obj.type = SourceType::Dir; // 目前传目录的只有AVS, AVS就特殊处理
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
                obj.sub_title = tv.sub_title_prefix + TEXT(" | ") + obj.sub_title;
            }
            return;
        }
    }
    obj.title_prefix = TEXT("");
}

void Publish::setResolution(int64_t width, int64_t height, const av::media::ScanType& scan_type, Source& obj) {
    obj.video_resolution = SourceVideoResolution::_1080p; // default
    if (height == 4320) {
        obj.video_resolution = SourceVideoResolution::_8k;
    }
    else if (height == 2160 || (width == 3840 && height >= 1500 && height <= 2160)) {
        obj.video_resolution = SourceVideoResolution::_4k;
    }
    else if (height > 720 && height <= 1080) {
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
    else if (height <= 720 && height > 480) {
        obj.video_resolution = SourceVideoResolution::_720;
    }
    else if (height <= 480) {
        obj.video_resolution = SourceVideoResolution::_480;
    }
}

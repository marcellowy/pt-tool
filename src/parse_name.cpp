#include "parse_name.h"

#include <regex>
#include <string>
#include <iostream>
#include <chrono>

#include "av_string.h"
#include "av_log.h"
#include "mteam/category.h"


bool parseCustomName(PublishObj& obj) {
    // 0000@[402][标题][标题前缀][副标题][年份][豆瓣id][剧集][英文名].ts
    std::regex r(av::str::toA(publishPrefixCustom) + "\\[(.*?)\\]\\[(.*?)\\]\\[(.*?)\\]\\[(.*?)\\]\\[(.*?)\\]\\[(.*?)\\]\\[(.*?)\\]\\[(.*?)\\]");
    std::smatch match;
    std::string start_tmp = av::str::toA(obj.name);
    std::string::const_iterator start = start_tmp.begin();
    std::string::const_iterator end = start_tmp.end();

    while (std::regex_search(start, end, match, r)) {
        logd("match size: {}", match.size());
        if (match.size() == 9) {
            try {
                obj.year = av::str::toT(match[5]);
                obj.name_chs = av::str::toT(match[2]);
                obj.sub_title = av::str::toT(match[4]);
                obj.title_prefix = av::str::toT(match[3]);
                obj.douban_id = av::str::toT(match[6]);
                obj.season = av::str::toT(match[7]);
                obj.name_eng = av::str::toT(match[8]);

                // category id
                auto tmp = av::str::toT(match[1]);
                obj.category_id = mteam::category::from_string(tmp);

                logi("year: {}", av::str::toA(obj.year));
                logi("name_chs: {}", av::str::toA(obj.name_chs));
                logi("sub_title: {}", av::str::toA(obj.sub_title));
                logi("title_prefix: {}", av::str::toA(obj.title_prefix));
                logi("douban_id: {}", av::str::toA(obj.douban_id));
                logi("category_id: {}", av::str::toA(mteam::category::to_string(obj.category_id)));
                logi("season: {}", av::str::toA(obj.season));
                logi("name_eng: {}", av::str::toA(obj.name_eng));

                return true;
            }
            catch (const std::exception& e) {
                std::cout << " exception: " << e.what() << std::endl;
                loge("exception {}", e.what());
                return false;
            }
        }
        start = match.suffix().first;
    }
    return false;
}

bool parseMovieName(PublishObj& obj) {
    // 一代妖后1989€1437318.ts
    std::regex r("\\s*(.*?)\\s*(\\d+)\\s*€\\s*(\\d+)\\s*\\.ts");
    std::smatch match;
    std::string start_tmp = av::str::toA(obj.name);
    std::string::const_iterator start = start_tmp.begin();
    std::string::const_iterator end = start_tmp.end();

    while (std::regex_search(start, end, match, r)) {
        logd("match size: {}", match.size());
        if (match.size() == 4) {
            try {
                obj.year = av::str::toT(match[2]);
                obj.name_chs = av::str::toT(match[1]);
                obj.sub_title = TEXT("");
                obj.title_prefix = TEXT("");
                obj.douban_id = av::str::toT(match[3]);

                logi("year: {}", av::str::toA(obj.year));
                logi("name_chs: {}", av::str::toA(obj.name_chs));
                logi("sub_title: {}", av::str::toA(obj.sub_title));
                logi("title_prefix: {}", av::str::toA(obj.title_prefix));
                logi("douban_id: {}", av::str::toA(obj.douban_id));

                return true;
            }
            catch (const std::exception& e) {
                std::cout << " exception: " << e.what() << std::endl;
                loge("exception {}", e.what());
                return false;
            }
        }
        start = match.suffix().first;
    }
    return false;
}

bool parseTVSeriesName(PublishObj& obj) {
    return false;
}

bool parseDiscoverName(PublishObj& obj) {
    // 000@嫦娥六号1(4K)_CCTV4K_04_11_17_57.ts
    std::regex r(av::str::toA(publishPrefixDiscover) + "(.*?)\\_(.*?)\\_([\\d]+)\\_([\\d]+)\\_[\\d]+\\_[\\d]+.ts");
    std::smatch match;
    std::string start_tmp = av::str::toA(obj.name);
    std::string::const_iterator start = start_tmp.begin();
    std::string::const_iterator end = start_tmp.end();

    while (std::regex_search(start, end, match, r)) {
        logd("match size: {}", match.size());
        if (match.size() == 5) {
            try {
                auto today = floor<std::chrono::days>(std::chrono::system_clock::now());
                std::chrono::year_month_day ymd = std::chrono::year_month_day{ today };
                obj.year = av::str::toT(std::to_string(int(ymd.year()))) + av::str::toT(match[3]) + av::str::toT(match[4]);
                obj.name_chs = av::str::toT(match[1]);
                obj.sub_title = obj.name_chs;
                obj.title_prefix = av::str::toT(match[2]);

                logi("year: {}", av::str::toA(obj.year));
                logi("name_chs: {}", av::str::toA(obj.name_chs));
                logi("sub_title: {}", av::str::toA(obj.sub_title));
                logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                return true;
            }
            catch (const std::exception& e) {
                std::cout << " exception: " << e.what() << std::endl;
                loge("exception {}", e.what());
                return false;
            }
        }
        start = match.suffix().first;
    }
    return false;
}

bool parseVarietyName(PublishObj& obj) {
    {
        // 00@边陲新风——全民国家安全教育日特别节目-1_CCTV12_04_15_20_37.ts
        std::regex r(av::str::toA(publishPrefixVariety) + "(.*?)_(.*?)_([\\d]{1,})_([\\d]{1,})_([\\d]{1,})_([\\d]{1,}).ts");
        std::smatch match;
        std::string start_tmp = av::str::toA(obj.name);
        std::string::const_iterator start = start_tmp.begin();
        std::string::const_iterator end = start_tmp.end();

        while (std::regex_search(start, end, match, r)) {
            logd("match size: {}", match.size());
            if (match.size() == 7) {
                try {
                    std::tstring tmp = av::str::toT(match[1]);
                    av::str::remove_suffix(tmp, TEXT("-1"));
                    av::str::remove_suffix(tmp, TEXT("-2"));
                    av::str::remove_suffix(tmp, TEXT("-3"));
                    auto today = floor<std::chrono::days>(std::chrono::system_clock::now());
                    std::chrono::year_month_day ymd = std::chrono::year_month_day{ today };
                    obj.year = av::str::toT(std::to_string(int(ymd.year()))) + av::str::toT(match[3]) + av::str::toT(match[4]);
                    obj.name_chs = tmp;
                    obj.sub_title = obj.name_chs;
                    obj.title_prefix = av::str::toT(match[2]);

                    logi("year: {}", av::str::toA(obj.year));
                    logi("name_chs: {}", av::str::toA(obj.name_chs));
                    logi("sub_title: {}", av::str::toA(obj.sub_title));
                    logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                    return true;
                }
                catch (const std::exception& e) {
                    std::cout << " exception: " << e.what() << std::endl;
                    loge("exception {}", e.what());
                    return false;
                }
            }
            start = match.suffix().first;
        }

    }

    {
        // 00@第27届上海国际电影节金爵奖颁奖典礼_06_21_20_00.ts
        std::regex r(av::str::toA(publishPrefixVariety) + "(.*?)_([\\d]{2})_([\\d]{2})_([\\d]{2})_([\\d]{2}).ts");
        std::smatch match;
        std::string start_tmp = av::str::toA(obj.name);
        std::string::const_iterator start = start_tmp.begin();
        std::string::const_iterator end = start_tmp.end();

        while (std::regex_search(start, end, match, r)) {
            logd("match size: {}", match.size());
            if (match.size() == 6) {
                try {
                    std::tstring tmp = av::str::toT(match[1]);
                    auto today = floor<std::chrono::days>(std::chrono::system_clock::now());
                    std::chrono::year_month_day ymd = std::chrono::year_month_day{ today };
                    obj.year = av::str::toT(std::to_string(int(ymd.year()))) + av::str::toT(match[2]) + av::str::toT(match[3]);
                    obj.name_chs = tmp;
                    obj.sub_title = obj.name_chs;
                    obj.title_prefix = TEXT("");

                    logi("year: {}", av::str::toA(obj.year));
                    logi("name_chs: {}", av::str::toA(obj.name_chs));
                    logi("sub_title: {}", av::str::toA(obj.sub_title));
                    logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                    return true;
                }
                catch (const std::exception& e) {
                    std::cout << " exception: " << e.what() << std::endl;
                    loge("exception {}", e.what());
                    return false;
                }
            }
            start = match.suffix().first;
        }

    }
    return false;
}

bool parseSportName(PublishObj& obj) {

    {
        // 0@2024-2025赛季中国男子篮球职业联赛-总决赛第四场(北京北汽-浙江方兴渡)_CCTV5_05_14_18_57.ts
        std::regex r(av::str::toA(publishPrefixSport) + "([0-9]{4})[\\-\\_]{1,}([0-9]{4})(.*?)\\_(CCTV[\\d]{1,}[\\+]{0,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\.ts");
        std::smatch match;
        std::string start_tmp = av::str::toA(obj.name);
        std::string::const_iterator start = start_tmp.begin();
        std::string::const_iterator end = start_tmp.end();

        while (std::regex_search(start, end, match, r)) {
            logd("match size: {}", match.size());
            if (match.size() == 9) {
                try {
                    obj.year = av::str::toT(match[2]) + av::str::toT(match[5]) + av::str::toT(match[6]);
                    obj.name_chs = av::str::toT(match[1]) + TEXT("-") + av::str::toT(match[2]) + av::str::toT(match[3]);
                    obj.sub_title = obj.name_chs;
                    obj.title_prefix = av::str::toT(match[4]);

                    logi("year: {}", av::str::toA(obj.year));
                    logi("name_chs: {}", av::str::toA(obj.name_chs));
                    logi("sub_title: {}", av::str::toA(obj.sub_title));
                    logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                    return true;
                }
                catch (const std::exception& e) {
                    std::cout << " exception: " << e.what() << std::endl;
                    loge("exception {}", e.what());
                    return false;
                }
            }
            start = match.suffix().first;
        }

    }

    {
        // 0@2025年世界泳联花样游泳世界杯(埃及站) - 双人自由自选决赛_CCTV5_04_12_16_57.ts
        std::regex r(av::str::toA(publishPrefixSport) + "([0-9]{4})(.*?)\\_(CCTV[\\d]{1,}[\\+]{0,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\.ts");
        std::smatch match;
        std::string start_tmp = av::str::toA(obj.name);
        std::string::const_iterator start = start_tmp.begin();
        std::string::const_iterator end = start_tmp.end();

        while (std::regex_search(start, end, match, r)) {
            logd("match size: {}", match.size());
            if (match.size() == 8) {
                try {
                    obj.year = av::str::toT(match[1]) + av::str::toT(match[4]) + av::str::toT(match[5]);
                    obj.name_chs = av::str::toT(match[1]) + TEXT("-") + av::str::toT(match[2]);
                    obj.sub_title = obj.name_chs;
                    obj.title_prefix = av::str::toT(match[3]);

                    logi("year: {}", av::str::toA(obj.year));
                    logi("name_chs: {}", av::str::toA(obj.name_chs));
                    logi("sub_title: {}", av::str::toA(obj.sub_title));
                    logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                    return true;
                }
                catch (const std::exception& e) {
                    std::cout << " exception: " << e.what() << std::endl;
                    loge("exception {}", e.what());
                    return false;
                }
            }
            start = match.suffix().first;
        }
    }

    {
        // 0@现场直播(高清体育) - 2025年十堰马拉松_CCTV5 + _04_13_07_22.ts
        std::regex r(av::str::toA(publishPrefixSport) + "(.*?)\\-([\\d]+)(.*?)\\_(CCTV[\\d]{1,}[\\+]{0,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\.ts");
        std::smatch match;
        std::string start_tmp = av::str::toA(obj.name);
        std::string::const_iterator start = start_tmp.begin();
        std::string::const_iterator end = start_tmp.end();

        while (std::regex_search(start, end, match, r)) {
            logd("match size: {}", match.size());
            if (match.size() == 9) {
                try {
                    obj.year = av::str::toT(match[2]) + av::str::toT(match[5]) + av::str::toT(match[6]);
                    obj.name_chs = av::str::toT(match[1]) + TEXT("-") + av::str::toT(match[2]) + av::str::toT(match[3]);
                    obj.sub_title = obj.name_chs;
                    obj.title_prefix = av::str::toT(match[4]);

                    logi("year: {}", av::str::toA(obj.year));
                    logi("name_chs: {}", av::str::toA(obj.name_chs));
                    logi("sub_title: {}", av::str::toA(obj.sub_title));
                    logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                    return true;
                }
                catch (const std::exception& e) {
                    std::cout << " exception: " << e.what() << std::endl;
                    loge("exception {}", e.what());
                    return false;
                }
            }
            start = match.suffix().first;
        }
    }

    {
        // 0@北京国际长跑节北京半程马拉松_北京卫视_04_20_06_59.ts
        std::regex r(av::str::toA(publishPrefixSport) + "(.*?)\\_(.*)\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\_([\\d]{1,})\\.ts");
        std::smatch match;
        std::string start_tmp = av::str::toA(obj.name);
        std::string::const_iterator start = start_tmp.begin();
        std::string::const_iterator end = start_tmp.end();

        while (std::regex_search(start, end, match, r)) {
            logd("match size: {}", match.size());
            if (match.size() == 7) {
                try {
                    auto today = floor<std::chrono::days>(std::chrono::system_clock::now());
                    std::chrono::year_month_day ymd = std::chrono::year_month_day{ today };
                    obj.year = av::str::toT(std::to_string(int(ymd.year()))) + av::str::toT(match[3]) + av::str::toT(match[4]);
                    obj.name_chs = av::str::toT(match[1]);
                    obj.sub_title = obj.name_chs;
                    obj.title_prefix = av::str::toT(match[2]);

                    logi("year: {}", av::str::toA(obj.year));
                    logi("name_chs: {}", av::str::toA(obj.name_chs));
                    logi("sub_title: {}", av::str::toA(obj.sub_title));
                    logi("title_prefix: {}", av::str::toA(obj.title_prefix));

                    return true;
                }
                catch (const std::exception& e) {
                    std::cout << " exception: " << e.what() << std::endl;
                    loge("exception {}", e.what());
                    return false;
                }
            }
            start = match.suffix().first;
        }
    }

    

    
    return false;
}

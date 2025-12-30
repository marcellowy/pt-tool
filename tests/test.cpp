#include "parse_name.h"
#include "defined.h"
#include <iostream>
#include "logger.h"

#define TestParseName true

int main(){
#if _WIN32
#include "Windows.h"
    SetConsoleOutputCP(CP_UTF8);
#endif // _WIN32

    if (!Logger::instance().open()) {
        return 0;
    }
    
    if (TestParseName)
    {
        {
            PublishObj obj;
            obj.name = TEXT("0@2024-2025赛季中国男子篮球职业联赛-总决赛第四场(北京北汽-浙江方兴渡)_CCTV5_05_14_18_57.ts");
            if (!parseSportName(obj)) {
                std::cout << "parseSportName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("0@2024_2025赛季北美冰球职业联赛-常规赛74（明尼苏达狂野-卡尔加里火焰）_CCTV5+_04_12_09_57.ts");
            if (!parseSportName(obj)) {
                std::cout << "parseSportName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("0@2025年世界泳联花样游泳世界杯(埃及站) - 双人自由自选决赛_CCTV5_04_12_16_57.ts");
            if (!parseSportName(obj)) {
                std::cout << "parseSportName failed" << std::endl;
            }
        }
    
        {
            PublishObj obj;
            obj.name = TEXT("0@现场直播(高清体育)-2025年十堰马拉松_CCTV5+_04_13_07_22.ts");
            if (!parseSportName(obj)) {
                std::cout << "parseSportName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("0@北京国际长跑节北京半程马拉松_北京卫视_04_20_06_59.ts");
            if (!parseSportName(obj)) {
                std::cout << "parseSportName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("一代妖后1989€1437318.ts");
            if (!parseMovieName(obj)) {
                std::cout << "parseMovieName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("一代妖后1989 €1437318.ts");
            if (!parseMovieName(obj)) {
                std::cout << "parseMovieName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("000@嫦娥六号1(4K)_CCTV4K_04_11_17_57.ts");
            if (!parseDiscoverName(obj)) {
                std::cout << "parseDiscoverName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("00@边陲新风——全民国家安全教育日特别节目-1_CCTV12_04_15_20_37.ts");
            if (!parseVarietyName(obj)) {
                std::cout << "parseVarietyName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("00@第27届上海国际电影节金爵奖颁奖典礼_06_21_20_00.ts");
            if (!parseVarietyName(obj)) {
                std::cout << "parseVarietyName failed" << std::endl;
            }
        }

        {
            PublishObj obj;
            obj.name = TEXT("0000@[402][标题][标题前缀][副标题][年份][豆瓣id][剧集][英文名].ts");
            if (!parseCustomName(obj)) {
                std::cout << "parseCustomName failed" << std::endl;
            }
        }
        
        
    }
    
}

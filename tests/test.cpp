//#include "parse_name.h"
//#include "logger.h"
//#include "gtest/gtest.h"
//#include "mteam/upload_img.h"
//#include "av_log.h"
//#include "av_http.h"
//#include "logger.h"
//#include "av_translate.h"
//#include "av_env.h"
//#include "config.h"
//
//class MteamTest : public ::testing::Test {
//protected:
//	void SetUp() override {
//		if (!Logger::instance().open()) {
//			std::cout << "can not open log" << std::endl;
//			return;
//		}
//
//		std::tstring config_file = TEXT("config.toml");
//		if (av::env::is_dev()) {
//			config_file = TEXT("config_dev.toml");
//		}
//		if (!Config::instance().parse(config_file)) {
//			loge("parse config.toml failed");
//			return ;
//		}
//	}
//
//	void TearDown() override {
//
//	}
//};
//
//
//TEST_F(MteamTest, DISABLED_UploadImage) {
//	auto& config = Config::instance();
//	std::tstring url;
//	mteam::UploadImg a(config.mteam.img_api_url, config.mteam.img_api_key);
//	if (!a.Upload(TEXT("test.jpeg"), url)) {
//		loge("upload failed");
//		return;
//	}
//	logi("upload url {}", av::str::toA(url));
//	return;
//}
//
//TEST_F(MteamTest, DISABLED_QbittorrentLogin) {
//	av::http::Client client;
//	av::http::Header h;
//	av::http::File f;
//	av::http::FormData d;
//	av::http::Response response;
//	d.data[TEXT("username")] = TEXT("admin");
//	d.data[TEXT("password")] = TEXT("marcello123");
//
//	auto aa = std::make_tuple(h, d);
//	if (!client.postForm(TEXT("http://192.168.50.205:8086/api/v2/auth/login"), d, response)) {
//		loge("http failed");
//		return;
//	}
//	logi("http response code {}\n body: {}", response.code, av::str::toA(response.body));
//
//	if (response.isOk()) {
//		logi("is ok");
//		for (auto& aa : response.header.data) {
//			logi("header {}: {}", av::str::toA(aa.first), av::str::toA(aa.second));
//		}
//		for (auto& cookie : response.header.cookie.data) {
//			logi("cookie {}: {}", av::str::toA(cookie.first), av::str::toA(cookie.second));
//		}
//	}
//}
//
//TEST_F(MteamTest, DISABLED_Translate) {
//	auto& config = Config::instance();
//	std::tstring text;
//	av::translate::Translate t(config.rapidapi.key,config.rapidapi.host);
//	if (!t.foo(TEXT("中国"), text)) {
//		loge("translate error");
//		return;
//	}
//	else {
//		logi("translate success {}", av::str::toA(text));
//	}
//}
//
//TEST_F(MteamTest, aa) {
//	//logi("{}, {}, {}", av::time::seconds(), av::time::milliseconds(), av::time::microseconds());
//	const std::vector<int64_t> tt = { 60, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 120 };
//	int64_t count = 0;
//
//	av::codec::StbPNG stbPng([&count](void* data, int size) {
//		//logi("capture freame callback, {}", count);
//		//char buff[1024];
//		//snprintf(buff, sizeof(buff) - 1, "test_%ll.png", count);
//
//		std::tstringstream oo;
//		oo << TEXT("test_") << count << TEXT(".png");
//
//		std::ofstream out_file(av::str::toA(oo.str()), std::ios::binary);
//		out_file.write(static_cast<char*>(data), size);  // 写入数据到文件
//		count++;
//		});
//}

//#define TestParseName true
//
//int main(){
//#if _WIN32
//#include "Windows.h"
//    SetConsoleOutputCP(CP_UTF8);
//#endif // _WIN32
//
//    if (!Logger::instance().open()) {
//        return 0;
//    }
//    
//    if (TestParseName)
//    {
//        {
//            Source obj;
//            obj.name = TEXT("0@2024-2025赛季中国男子篮球职业联赛-总决赛第四场(北京北汽-浙江方兴渡)_CCTV5_05_14_18_57.ts");
//            if (!parseSportName(obj)) {
//                std::cout << "parseSportName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("0@2024_2025赛季北美冰球职业联赛-常规赛74（明尼苏达狂野-卡尔加里火焰）_CCTV5+_04_12_09_57.ts");
//            if (!parseSportName(obj)) {
//                std::cout << "parseSportName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("0@2025年世界泳联花样游泳世界杯(埃及站) - 双人自由自选决赛_CCTV5_04_12_16_57.ts");
//            if (!parseSportName(obj)) {
//                std::cout << "parseSportName failed" << std::endl;
//            }
//        }
//    
//        {
//            Source obj;
//            obj.name = TEXT("0@现场直播(高清体育)-2025年十堰马拉松_CCTV5+_04_13_07_22.ts");
//            if (!parseSportName(obj)) {
//                std::cout << "parseSportName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("0@北京国际长跑节北京半程马拉松_北京卫视_04_20_06_59.ts");
//            if (!parseSportName(obj)) {
//                std::cout << "parseSportName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("一代妖后1989€1437318.ts");
//            if (!parseMovieName(obj)) {
//                std::cout << "parseMovieName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("一代妖后1989 €1437318.ts");
//            if (!parseMovieName(obj)) {
//                std::cout << "parseMovieName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("000@嫦娥六号1(4K)_CCTV4K_04_11_17_57.ts");
//            if (!parseDiscoverName(obj)) {
//                std::cout << "parseDiscoverName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("00@边陲新风——全民国家安全教育日特别节目-1_CCTV12_04_15_20_37.ts");
//            if (!parseVarietyName(obj)) {
//                std::cout << "parseVarietyName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("00@第27届上海国际电影节金爵奖颁奖典礼_06_21_20_00.ts");
//            if (!parseVarietyName(obj)) {
//                std::cout << "parseVarietyName failed" << std::endl;
//            }
//        }
//
//        {
//            Source obj;
//            obj.name = TEXT("0000@[402][标题][标题前缀][副标题][年份][豆瓣id][剧集][英文名].ts");
//            if (!parseCustomName(obj)) {
//                std::cout << "parseCustomName failed" << std::endl;
//            }
//        }
//        
//        
//    }
//    
//}

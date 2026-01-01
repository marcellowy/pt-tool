// test.cpp: 定义应用程序的入口点。
//


#include "test.h"

#include <curl/curl.h>

//#include "httplib/httplib.h"
//#include "cron.h"
#include "av_log.h"
#include "config.h"
#include "logger.h"
#include "error_code.h"
//#include "av_async.h"
//#include "av_queue.h"
//#include "av_base64.h"
#include "publish.h"
#include "av_path.h"
#include "av_time.h"
#include "av_mediainfo.h"
#include "av_ffmpeg.h"
#include "av_codec_jpg.h"
#include "av_codec_stb_image_jpg.h"
#include "av_translate.h"
#include "av_env.h"
#include "av_async.h"
#include "av_http.h"

using namespace std;

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	curl_global_init(CURL_GLOBAL_DEFAULT);
	av::async::Exit exit_curl_g([] {
		curl_global_cleanup();
	});

	if (!Logger::instance().open()) {
		return ErrorCode::ErrOpenLogFailed;
	}

	std::tstring config_file = TEXT("config.toml");
	if (av::env::is_dev()) {
		config_file = TEXT("config_dev.toml");
	}
	if (!Config::instance().parse(config_file)) {
		loge("parse config.toml failed");
		return ErrorCode::ErrParseConfigFileFailed;
	}
	auto& config = Config::instance();
	
	logi("server start ==================================");

	av::http::Client client;
	av::http::Response response;
	/*if (!client.get(TEXT("https://www.baidu.com"), response)) {
		loge("http failed");

	}*/

	av::http::Header h;
	av::http::File f;
	av::http::FormData d;
	d.data[TEXT("username")] = TEXT("admin");
	d.data[TEXT("password")] = TEXT("marcello123");

	auto aa = std::make_tuple(h, d);
	if (!client.postForm(TEXT("http://192.168.50.205:8086/api/v2/auth/login"), d, response)) {
		loge("http failed");
		return 0;
	}
	logi("http response code {}\n body: {}", response.code, av::str::toA(response.body));

	if (response.isOk()) {
		logi("is ok");
		for (auto& aa : response.header.data) {
			logi("header {}: {}", av::str::toA(aa.first), av::str::toA(aa.second));
		}
		for (auto& cookie: response.header.cookie.data) {
			logi("cookie {}: {}", av::str::toA(cookie.first), av::str::toA(cookie.second));
		}
	}

	return 0;

	//logi("{}, {}, {}", av::time::seconds(), av::time::milliseconds(), av::time::microseconds());
	const std::vector<int64_t> tt = { 60, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 120 };
	int64_t count = 0;

	av::codec::StbPNG stbPng([&count](void* data, int size) {
		//logi("capture freame callback, {}", count);
		//char buff[1024];
		//snprintf(buff, sizeof(buff) - 1, "test_%ll.png", count);

		std::tstringstream oo;
		oo << TEXT("test_") << count << TEXT(".png");

		std::ofstream out_file(av::str::toA(oo.str()), std::ios::binary);
		out_file.write(static_cast<char*>(data), size);  // 写入数据到文件
		count++;
	});

	//std::tstring text;
	//av::translate::Translate t(config.rapidapi.key,config.rapidapi.host);
	//if (!t.foo(TEXT("中国"), text)) {
	//	loge("translate error");
	//	return 0;
	//}
	//else {
	//	logi("translate success {}", av::str::toA(text));
	//}

#ifdef _WIN32

	const char* a = std::getenv("PT_TOOL_ENV");
	if (a == NULL) {
		logi("no value");
	}
	else {
		logi("MY_USER_VARIABLE {}", a);
	}
	

	
	av::mediainfo::MediaInfo m(TEXT("C:/Users/Marcello/Downloads/1153734.mp4"));
	if (!m.parse()) {
		loge("parse mediainfo failed");
	}

	av::codec::JPG jpg([&count](uint8_t* data, size_t size) {
		logi("capture freame callback, {}", count);
		// save
		char filename[128];
		sprintf(filename, "frame%lld.jpg", count);
		FILE* f = fopen(filename, "wb");
		if (f == NULL) {
			loge("open {} failed", av::str::toA(filename));
			return;
		}
		fwrite(data, 1, size, f);
		count++;
	});

	if (!av::ffmpeg::captureFrame(TEXT("C:\\Users\\Marcello\\Downloads\\中.mp4"), tt, stbPng)) {
		loge("captureFrame failed!!!");
	}

	Publish p(av::str::toT("D:\\Downloads\\tmp - 复制"));
#else

	av::mediainfo::MediaInfo m("/home/marcello/tmp/中.mp4");
	if (!m.parse()) {
		loge("parse mediainfo failed");
	}

	if (!av::ffmpeg::captureFrame(TEXT("/home/marcello/tmp/中.mp4"), tt, stbPng)) {
		loge("captureFrame failed!!!");
	}

	Publish p(av::str::toT("/home/marcello/tmp"));
#endif
	//p.start();

	//Queue<int> q;
	//Cron a("1", "*1 * * * * *", [&q] {
	//	av::async::Exit exit_cron([&q] {
	//		logi("test aa end base64: {}", av::base64::encode("1234567890"));
	//		q.push(2);
	//		});
	//	q.push(1);
	//	});
	//a.start();

	//auto tmp_thread = std::thread([&a, &q] {
	//	std::this_thread::sleep_for(std::chrono::seconds(10));
	//	a.stop();
	//	q.close();
	//});

	//auto tmp_thread2 = std::thread([&q] {
	//	int a = 0;
	//	while (q.pop(a)) {
	//		logi("THREAD2 q val {}", a);
	//	}
	//	logi("q closed pop thread exit");
	//});

	//auto tmp_thread3 = std::thread([&q] {
	//	int a = 0;
	//	while (q.pop(a)) {
	//		logi("THREAD3 q val {}", a);
	//	}
	//	logi("q closed pop thread exit");
	//	});

	//httplib::Server svr;

	//svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
	//	res.set_content("123", "text/plain");
	//});

	//// check
	//if (config.server.host == "" || config.server.port == 0) {
	//	loge("server param error {}:{}", config.server.host, config.server.port);
	//	return ErrorCode::ErrServerParam;
	//}

	//// bind
	//int port = svr.bind_to_port(config.server.host, config.server.port);
	//if (port < 0) {
	//	loge("bind_to_port {}:{} failed", config.server.host, config.server.port);
	//	return ErrorCode::ErrServerBind;
	//}

	//// listen
	//if (!svr.listen_after_bind()) {
	//	loge("listen_after_bind {}:{} failed", config.server.host, config.server.port);
	//	return ErrorCode::ErrServerListen;
	//}
	return ErrorCode::Success;
}
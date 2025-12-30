// test.cpp: 定义应用程序的入口点。
//

#include "av_base.h"
#include "test.h"
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

using namespace std;

int main()
{
//#ifdef UNICODE
//	logi("UNICODE");
//#elifdef _UNICODE
//	logi("_UNICODE");
//#else
//	logi("NOT UNICODE");
//#endif // UNICODE

#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	if (!Logger::instance().open()) {
		return ErrorCode::ErrOpenLogFailed;
	}

	logi("中文测试");

	if (!Config::instance().parse(av::str::toT("config.toml"))) {
		loge("parse config.toml failed");
		return ErrorCode::ErrParseConfigFileFailed;
	}
	auto& config = Config::instance();
	
	logi("server start ==================================");

	av::path::create_dir(TEXT("中文目录"));
	av::path::create_dir(TEXT("中文目录1"));
	av::path::remove_dir(TEXT("中文目录1"));
	
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
#ifdef _WIN32

	av::mediainfo::MediaInfo m(TEXT("C:/Users/Marcello/Downloads/中.mp4"));
	if (!m.parse()) {
		loge("parse mediainfo failed");
	} else{
		logi("video format {} w: {} h: {}",av::str::toA(m.getVideo().format), m.getVideo().width, m.getVideo().height);
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
	else {
		logi("video format {} w: {} h: {}", av::str::toA(m.getVideo().format), m.getVideo().width, m.getVideo().height);
	}

	if (!av::ffmpeg::captureFrame(TEXT("/home/marcello/tmp/中.mp4"), tt, stbPng)) {
		loge("captureFrame failed!!!");
	}

	Publish p(av::str::toT("/home/marcello/tmp"));
#endif
	//p.start();

	//Queue<int> q;
	//Cron a("1", "*/1 * * * * *", [&q] {
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
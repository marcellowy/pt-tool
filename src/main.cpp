// test.cpp: 定义应用程序的入口点。
//

#include <csignal>
#include <atomic>

#include "main.h"
#include <curl/curl.h>
#include "httplib.h"

#include "av_async.h"
#include "av_env.h"
#include "av_log.h"
#include "av_worker.h"

#include "config.h"
#include "logger.h"
#include "error_code.h"

#include "publish.h"
#include "download.h"
#include "mteam/mteam.h"

using namespace std;

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	curl_global_init(CURL_GLOBAL_DEFAULT);
	av::async::Exit exit([] {

		// 停止curl
		curl_global_cleanup();

		// 停止全局工作线程
		av::worker::shutdown_global_worker();
	});

	if (!Logger::instance().open()) {
		return static_cast<int>(ErrorCode::ErrOpenLogFailed);
	}

	std::tstring config_file = TEXT("config.toml");
	if (!Config::instance().parse(config_file)) {
		loge("parse config.toml failed");
		return static_cast<int>(ErrorCode::ErrParseConfigFileFailed);
	}
	auto& config = Config::instance();
	logi("server start ==================================");

	std::shared_ptr<Site> ptr = std::make_shared<mteam::Mteam>(
		config.mteam.api_url, config.mteam.api_key,
		config.mteam.img_api_url, config.mteam.img_api_key,
		config.tgbot.token, config.tgbot.chat_id
	);
	Publish publish(ptr, config.mteam.seed_dir);
	if (!publish.start()) {
		logw("start publish cron failed");
		return static_cast<int>(ErrorCode::ErrStartCronFailed);
	}
	//publish.task();

	//
	Download download;
	if (!download.start()) {
		logw("start download cron failed");
		return static_cast<int>(ErrorCode::ErrStartCronFailed);
	}

	//
	httplib::Server svr;
	if (!svr.bind_to_port(av::str::toA(config.server.host), config.server.port)) {
		logw("bind failed");
		return static_cast<int>(ErrorCode::ErrOpenLogFailed);
	}
	svr.listen_after_bind();

	return static_cast<int>(ErrorCode::Success);
}

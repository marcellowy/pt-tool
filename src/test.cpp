// test.cpp: 定义应用程序的入口点。
//


#include "test.h"
#include <curl/curl.h>

#include "av_async.h"
#include "av_env.h"
#include "av_log.h"

#include "config.h"
#include "logger.h"
#include "error_code.h"

#include "publish.h"
#include "mteam/mteam.h"

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

	std::shared_ptr<Site> ptr = std::make_shared<mteam::Mteam>();
	Publish publish(ptr, TEXT("D:\\Downloads\\tmp2"));
	publish.start();
	


	return ErrorCode::Success;
}

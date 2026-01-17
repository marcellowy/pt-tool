#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include  "src/publish.h"
#include <csignal>
#include <atomic>

#include <curl/curl.h>
#include "httplib.h"

#include "av_async.h"
#include "av_env.h"
#include "av_log.h"

#include "src/config.h"
#include "logger.h"
#include "src/error_code.h"

#include "src/publish.h"
#include "mteam/mteam.h"

class PublishTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
			return;
		}

		std::tstring config_file = TEXT("config.toml");
		if (av::env::is_dev()) {
			config_file = TEXT("config_dev.toml");
		}
		if (!Config::instance().parse(config_file)) {
			loge("parse config.toml failed");
			return;
		}
	}

	void TearDown() override {

	}
};

TEST_F(PublishTest, task) {
	#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	curl_global_init(CURL_GLOBAL_DEFAULT);
	av::async::Exit exit_curl_g([] {
		curl_global_cleanup();
	});

	if (!Logger::instance().open()) {
		return ;
	}

	std::tstring config_file = TEXT("config.toml");
	if (!Config::instance().parse(config_file)) {
		loge("parse config.toml failed");
		return ;
	}
	auto& config = Config::instance();
	logi("server start ==================================");

	std::shared_ptr<Site> ptr = std::make_shared<mteam::Mteam>(
		config.mteam.api_url, config.mteam.api_key,
		config.mteam.img_api_url, config.mteam.img_api_key,
		config.tgbot.token, config.tgbot.chat_id
	);
	Publish publish(ptr, config.mteam.seed_dir);
	publish.task();
	std::abort();
}
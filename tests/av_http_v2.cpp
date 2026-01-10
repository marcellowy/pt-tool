#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "av_http_v2.h"

class AVHttpV2Test : public ::testing::Test {
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

TEST_F(AVHttpV2Test, get) {
	av::http_v2::Client c;
	av::http_v2::Response res;
	if (c.get("https://www.baidu.com/a/b/c?a=1&b=2", res)) {
		logi("response status code {}", res.status_code);
		logi("response body {}", res.body);
		return;
	}
	logw("get failed");
	
	std::abort();
}

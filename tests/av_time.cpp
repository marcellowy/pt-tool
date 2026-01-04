#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include <chrono>

#include "av_time.h"

class AVTimeTest : public ::testing::Test {
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

TEST_F(AVTimeTest, DISABLED_time_diff) {
	
	std::chrono::system_clock::duration d;
	auto a = av::time::diff_now("2026-01-04 18:00:00", d);
	std::chrono::duration seconds = std::chrono::duration_cast<std::chrono::seconds>(d);

	logi("secods: {}", seconds.count() / 60);
}

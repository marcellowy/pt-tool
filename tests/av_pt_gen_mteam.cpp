#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_pt_gen_mteam.h"


class AVPTGenMteamTest : public ::testing::Test {
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

TEST_F(AVPTGenMteamTest, douban) {
	auto& config = Config::instance();
	av::ptgen::Douban d;
	auto a = av::ptgen::getByMteam(TEXT("https://movie.douban.com/subject/10736442/"),
		config.mteam.api_key,d);
	if (!a) {
		loge("get douban failed");
	}
	std::abort();
}
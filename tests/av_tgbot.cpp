#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_tgbot.h"

class TGbotTest : public ::testing::Test {
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

TEST_F(TGbotTest, DISABLED_send) {
//TEST_F(TGbotTest, send) {
	// tgbot.NewBot("7772061300:AAEnZOQDCoxB01oZsdaEQDzd7iElOBw2xn8", 5607636949)
	// 获取chat_id
	// https://api.telegram.org/bot8022060026:AAG8bFnVE1H_OI0OaZRJu7vg_6htLnet1VI/getUpdates
	// bot nezha 可以找botfather查
	av::tgbot::send_message("8022060026:AAG8bFnVE1H_OI0OaZRJu7vg_6htLnet1VI", 5607636949, "测试消息");
}

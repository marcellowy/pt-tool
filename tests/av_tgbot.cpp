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

TEST_F(TGbotTest, send) {
//TEST_F(TGbotTest, send) {
	// 获取chat_id
	// https://api.telegram.org/bot{your token}/getUpdates
	// bot nezha 可以找botfather查
	auto& config = Config::instance();
	//av::tgbot::send_message(av::str::toA(config.tgbot.token), av::str::toA(config.tgbot.chat_id), "测试消息");
	auto img = "E:\\2025\\pt-tool\\out\\build\\x64-debug\\screenshots\\6e3602205c35797e91bf663e0760c180\\screenshot_2.png";
	av::tgbot::send_local_photo_message(av::str::toA(config.tgbot.token),
		av::str::toA(config.tgbot.chat_id),img, "测试消息111");

	/*av::tgbot::send_net_photo_message(av::str::toA(config.tgbot.token),
		av::str::toA(config.tgbot.chat_id), 
		av::str::toA("https://img3.doubanio.com/view/photo/l/public/p2928322397.webp"), "测试消息111");*/
	
	std::abort();

}

#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_tgbot.h"

//#define OPEN_TEST

#ifdef OPEN_TEST
#define DISABLED_(a) a
#else
#define DISABLED_(a) DISABLED_##a
#endif // OPEN_TEST

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

TEST_F(TGbotTest, DISABLED_(send_message)) {
	auto& config = Config::instance();
	av::tgbot::send_message(av::str::toA(config.tgbot.token),
		av::str::toA(config.tgbot.chat_id), "这是普通消息");
}

TEST_F(TGbotTest, DISABLED_(send_local_photo)) {
	auto& config = Config::instance();
	auto img = "C:\\Users\\chadwang\\Downloads\\Telegram Desktop\\tg_image_914412118.png";
	av::tgbot::send_local_photo_message(av::str::toA(config.tgbot.token),
		av::str::toA(config.tgbot.chat_id), img, "这是本地图片");
}

TEST_F(TGbotTest, DISABLED_(send_net_photo)) {
	// 获取chat_id
	// https://api.telegram.org/bot{your token}/getUpdates
	
	// bot nezha 可以找botfather查
	auto& config = Config::instance();
	//av::tgbot::send_message(av::str::toA(config.tgbot.token), av::str::toA(config.tgbot.chat_id), "测试消息");

	av::tgbot::send_net_photo_message(av::str::toA(config.tgbot.token),
		av::str::toA(config.tgbot.chat_id), 
		av::str::toA("https://img3.doubanio.com/view/photo/l/public/p2928322397.webp"), "这是网络图片");
	
	std::abort();

}


TEST_F(TGbotTest, DISABLED_(write_byte)) {
	// 获取chat_id
	// https://api.telegram.org/bot{your token}/getUpdates

	// bot nezha 可以找botfather查
	auto& config = Config::instance();
	//av::tgbot::send_message(av::str::toA(config.tgbot.token), av::str::toA(config.tgbot.chat_id), "测试消息");

	/*av::tgbot::send_net_photo_message(av::str::toA(config.tgbot.token),
		av::str::toA(config.tgbot.chat_id),
		av::str::toA("https://img3.doubanio.com/view/photo/l/public/p2928322397.webp"), "这是网络图片");*/

	char buff[1024];
	snprintf(buff, sizeof(buff), "1,2,3 %s", "aaa");

	logi("{}", av::str::toA(buff));

	std::string test;
	test.resize(1024);
	snprintf(test.data(), test.size() - 1, "1,2,3 %s", "aaa");
	//test.resize(strlen(test.c_str()));

	logi("{}", av::str::toA(test));

	std::abort();

}

#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_translate.h"

class TranslateTest : public ::testing::Test {
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

TEST_F(TranslateTest, foo) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	auto& config = Config::instance().rapidapi;
	av::translate::Translate t(config.key, config.host);
	std::tstring cc;
	std::tstring mm = TEXT("中国");
	if (!t.foo(mm, cc)) {
		loge("translate failed");
	}
	logi("cc {}", av::str::toA(cc));
	std::abort();

}

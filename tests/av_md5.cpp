#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"


#include "av_md5.h"

class AVMd5Test : public ::testing::Test {
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

//TEST_F(AVMd5Test, DISABLED_md5) {
TEST_F(AVMd5Test, md5) {
	std::string a = "123";
	std::string hash;
	av::hash::md5(a, hash);
	logi("{}", hash);
	//std::abort();
}

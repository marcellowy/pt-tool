#include "gtest/gtest.h"
#include "av_env.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "../logger.h"

#include  "douban/av_douban_img.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class AVDouBanImg : public ::testing::Test {
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

TEST_F(AVDouBanImg, get_img) {
	std::tstring url = TEXT("https://img3.doubanio.com/view/photo/s_ratio_poster/public/p2883995667.webp");
	fs::path p = url;
	fs::path file = p.filename();
	av::douban::get_img(url, [&file](const char* buff, size_t size)->void {
		logi("get img size {}", size);
		std::ofstream ofs;
		ofs.open(file, std::ios::binary | std::ios::trunc | std::ios::out);
		ofs.write(buff, size);
		ofs.close();
	});

	std::abort();
}

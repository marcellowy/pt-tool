#include "gtest/gtest.h"
#include "logger.h"
#include "src/config.h"
#include "av_env.h"
#include "boost/locale.hpp"
#include "av_path.h"
#include "av_file.h"

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

class TestReadfileTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
			return;
		}

		std::tstring config_file = TEXT("config.toml");
		if (!Config::instance().parse(config_file)) {
			loge("parse config.toml failed");
			return;
		}
	}

	void TearDown() override {
	}
};

TEST_F(TestReadfileTest, read) {
	std::tstring p = TEXT("D:\\Downloads\\media_info中.json");
	fs::path p1 = p;
	std::string mm;
	if (!av::file::read(p1, mm)) {
		logw("read file err");
		return;
	}
	logi("file content {}", mm);
	logw("read file succ");
}

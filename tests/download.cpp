#include "gtest/gtest.h"
#include "logger.h"
#include "src/download.h"
#include "src/config.h"

class DownloadTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
		}
	}

	void TearDown() override {
	}
};

TEST_F(DownloadTest, custom) {
	auto& config = Config::instance();
	config.download.numberOfDownloadPerTime = 100;
	config.download.enable = true;

	//
	Download d;
	d.task(TEXT("F:\\test1"), TEXT("F:\\test2"));
}

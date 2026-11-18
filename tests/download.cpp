#include "gtest/gtest.h"
#include "logger.h"
#include "src/download.h"
#include "src/config.h"
#include "av_log.h"

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

TEST_F(DownloadTest, file) {
	auto& config = Config::instance();
	config.download.numberOfDownloadPerTime = 1;
	config.download.enable = true;

	//
	Download d;
	d.task(TaskProcessType::kFile, TEXT("/root/media"), TEXT("/root/downloads"));
}

TEST_F(DownloadTest, dir) {
	auto& config = Config::instance();
	config.download.numberOfDownloadPerTime = 1;
	config.download.enable = true;

	//
	Download d;
	d.task(TaskProcessType::kDir, TEXT("/root/media"), TEXT("/root/downloads"));
}

TEST_F(DownloadTest, space) {
	std::error_code ec;
	fs::space_info si = fs::space("/root/downloads", ec);
	if (ec) {
		std::cerr << "get disk space failed: " << ec.message() << std::endl;
	}
	constexpr auto G = 1024 * 1024 * 1024;
	logi("path: {}, total size: {} GB, free size: {} GB, available size (non-privileged): {} GB",
		av::str::toA("/root/downloads"), si.capacity / G, si.free / G, si.available / G);
}

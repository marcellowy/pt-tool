#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_utils.h"

class AVUtilsTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
			return;
		}
		if (!Config::instance().parse(TEXT("config.toml"))) {
			loge("parse config.toml failed");
			return;
		}
	}

	void TearDown() override {

	}
};

TEST_F(AVUtilsTest, retry_backoff) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	auto a = std::chrono::steady_clock::now();
	av::utils::retry_backoff([]() -> int {
		throw std::runtime_error("aaa");
	}, 10,1000);

	auto b = std::chrono::steady_clock::now();

	logi("{}", std::chrono::duration_cast<std::chrono::milliseconds>(b-a).count() );

	std::abort();

}

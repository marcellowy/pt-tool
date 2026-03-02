#include "gtest/gtest.h"
#include "logger.h"
#include "src/parse_name.h"

class ParseNameTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
		}
	}

	void TearDown() override {
	}
};

TEST_F(ParseNameTest, custom) {
	Source obj;
	obj.name = TEXT("0000@[402][标题][标题前缀][副标题][年份][豆瓣id][剧集][英文名].ts");
	if (!parseCustomName(obj)) {
		logw("parse failed");
	}
	logi("category {}", static_cast<int>(obj.category));
}

#include "gtest/gtest.h"
#include "av_mediainfo.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"

class MediainfoTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
			return;
		}

		//std::tstring config_file = TEXT("config.toml");
		//if (av::env::is_dev()) {
		//	config_file = TEXT("config_dev.toml");
		//}
		//if (!Config::instance().parse(config_file)) {
		//	loge("parse config.toml failed");
		//	return;
		//}
	}

	void TearDown() override {

	}
};

TEST_F(MediainfoTest, Parse) {
	av::mediainfo::MediaInfo m(TEXT("C:\\Users\\Marcello\\Downloads\\pt-tool\\巴霍巴利王-终结 2017€26420932.ts"));
	if (!m.parse()) {
		loge("parse mediainfo failed");
	}
	logi("{}", static_cast<int>(m.getAudio().codec));
	logi("{}", av::str::toA(m.getText()));
	std::abort();

}

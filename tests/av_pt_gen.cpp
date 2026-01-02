#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#define DEBUG_PT_GEN
#include "av_pt_gen.h"

class PTGenTest : public ::testing::Test {
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

//TEST_F(PTGenTest, DISABLED_pt_gen) {
TEST_F(PTGenTest, pt_gen) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	std::string tmp;
	tmp.resize(4096);
	snprintf(tmp.data(), tmp.size() - 1, "https://pt-gen.lovewy123.workers.dev/?url=https%%3A%%2F%%2Fmovie.douban.com%%2Fsubject%%2F%s%%2F%%3Ffrom%%3Dshowing", 
		"35235151");

	std::tstring a = av::str::toT(tmp);
	av::ptgen::Douban d;
	if (!av::ptgen::get(a, d)) {
		loge("get error");
		return;
	}
	logi("{}\n{}\n{}\n{}\n{}\n{}\n{}", 
		d.name_chs, d.name_eng, d.year, d.sub_title, d.poster_img, d.imdb_link, d.description);
}




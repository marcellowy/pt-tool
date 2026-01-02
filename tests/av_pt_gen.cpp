#include "av_pt_gen.h"
#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

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

TEST_F(PTGenTest, DISABLED_pt_gen) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
	std::string tmp;
	tmp.resize(4096);
	snprintf(tmp.data(), tmp.size() - 1, "https://pt-gen.lovewy123.workers.dev/?url=https%%3A%%2F%%2Fmovie.douban.com%%2Fsubject%%2F%s%%2F%%3Ffrom%%3Dshowing", "37247238");

	std::tstring a = av::str::toT(tmp);
	av::ptgen::Data d;
	if (!av::ptgen::get(a, d)) {
		loge("get error");
		return;
	}
	logi("version {}", d.version);
	logi("chinese title {}", d.chinese_title);
	//logi("format {}", d.format);
	logi("code {}", d.code);
	logi("success {}", d.success);
	logi("message {}", d.message);
	logi("year {}", d.year);
	logi("imdb_id {}", d.imdb_id);
	logi("imdb_link {}", d.imdb_link);
	logi("poster {}", d.poster);
	//logi("{}", fmt::join(d.director, ","));
	//logi("{}", fmt::join(d.cast, ","));
	std::vector<std::string> directors;
	for (auto tmp: d.director) {
		directors.push_back(tmp.name);
	}
	std::vector<std::string> casts;
	for (auto tmp : d.cast) {
		casts.push_back(tmp.name);
	}
	logi("directors: {}", fmt::join(directors, ","));
	logi("casts: {}", fmt::join(casts, ","));
	logi("region: {}", fmt::join(d.region, ","));
	logi("genre: {}", fmt::join(d.genre, ","));
	logi("trans_title: {}", fmt::join(d.trans_title, ","));
	logi("this_title: {}", fmt::join(d.this_title, ","));
	
	
}




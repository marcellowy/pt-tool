#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "av_http.h"

class AVHttpTest : public ::testing::Test {
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

TEST_F(AVHttpTest, post_form) {
	av::http::Client client;
	av::http::Response resp;
	av::http::Header header;
	header.data[TEXT("User-Agent")] = TEXT("M-team TPTV PT Tools");
	header.data[TEXT("Content-type")] = TEXT("application/json");
	av::http::FormData form;
	if (!client.postForm(TEXT("https://api.m-team.cc/api/torrent/createOredit"), 
		std::make_tuple(header, form), resp)) {
		loge("");
	}

	std::abort();
}

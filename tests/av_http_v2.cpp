#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "av_http_v2.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class AVHttpV2Test : public ::testing::Test {
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

TEST_F(AVHttpV2Test, get) {
	av::http_v2::Client c;
	av::http_v2::Response res;
	if (c.get("https://www.baidu.com/a/b/c?a=1&b=2", res)) {
		logi("response status code {}", res.status);
		logi("response body {}", res.body);
		return;
	}
	logw("get failed");
	
	std::abort();
}

TEST_F(AVHttpV2Test, post_form) {
	av::http_v2::Client c;
	av::http_v2::Form form;
	form.kv.insert({"username", "admin"});
	form.kv.insert({"password", "marcello123" });
	form.file.insert({"ff", "C:\\Users\\Marcello\\Downloads\\1154742.mp4"});
	av::http_v2::Response resp;
	if (c.post("http://127.0.0.1:8000/hello", form, resp)) {
		if (resp.status == 200) {
			logi("response body {}", resp.body);
			logi("response status code {}", resp.status);
		}
		return;
	}

	logw("get failed");

	std::abort();
}

TEST_F(AVHttpV2Test, post_raw) {
	av::http_v2::Client c;

	// header
	av::http_v2::Header header;
	header.kv.insert({"Content-type", "application/json"});

	// json content
	json j;
	j["test_int"] = 1;
	j["test_str"] = "ss";
	std::string b = j.dump();

	//
	av::http_v2::Response resp;

	// send
	if (c.post("http://127.0.0.1:8000/hello", header, b, resp)) {
		if (resp.status == 200) {
			logi("response body {}", resp.body);
			logi("response status code {}", resp.status);
		}
		return;
	}

	logw("get failed");

	std::abort();
}

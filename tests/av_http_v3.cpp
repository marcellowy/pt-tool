#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "av_http_v3.h"
#include "av_http_v2.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace http = av::http_v2;

class AVHttpV3Test : public ::testing::Test {
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
		}
	}

	void TearDown() override {
	}
};

TEST_F(AVHttpV3Test, get) {
	http::Client c;
	const auto resp = c.get("https://www.bilibili.com");
	if (!resp) {
		loge("send http request failed");
		return;
	}
	for (auto &aa: resp->header.kv) {
		logi("response header {}: {}", aa.first, aa.second);
	}
	http::Cookie cookie;
	http::parseCookie(resp->header, cookie);
	for (auto &aa: cookie.kv) {
		logi("cookie {}: {}", aa.first, aa.second);
	}
	for (auto &aa: cookie.val) {
		logi("cookie {}", aa);
	}

	logi("status {}\n{}", resp->status, resp->body);
	std::abort();
}

TEST_F(AVHttpV3Test, post) {
	http::Client client;
	std::tstring url = TEXT("http://192.168.50.205:8086/api/v2/auth/login");
	http::Form form;
	form.kv["username"] = "admin";
	form.kv["password"] = "marcello123";
	const auto resp = client.post(av::str::toA(url), form);
	if (!resp) {
		loge("http request error");
		return;
	}
	if (resp->status == 200) {
		logi("is ok");
		for (auto &aa: resp->header.kv) {
			logi("response header {}: {}", aa.first, aa.second);
		}
		http::Cookie cookie;
		http::parseCookie(resp->header, cookie);
		for (auto &aa: cookie.kv) {
			logi("cookie {}: {}", aa.first, aa.second);
		}
		for (auto &aa: cookie.val) {
			logi("cookie {}", aa);
		}
		logi("body {}", resp->body);
	}
}

TEST_F(AVHttpV3Test, file_upload) {
	http::Client client;
	const std::tstring url = TEXT("http://127.0.0.1:8000/hello");
	http::Form form;
	form.kv["username"] = "admin";
	form.kv["password"] = "marcello123";
	form.file["ff"] = "11.txt";
	const auto resp = client.post(av::str::toA(url), form);
	if (!resp) {
		loge("http request error");
		return;
	}
	if (resp->status == 200) {
		logi("is ok");
		for (auto &aa: resp->header.kv) {
			logi("response header {}: {}", aa.first, aa.second);
		}
		http::Cookie cookie;
		http::parseCookie(resp->header, cookie);
		for (auto &aa: cookie.kv) {
			logi("cookie {}: {}", aa.first, aa.second);
		}
		for (auto &aa: cookie.val) {
			logi("cookie {}", aa);
		}
		logi("body {}", resp->body);
	}
}

#include <cctype>
#include <vector>
#include <regex>
#include <fstream>
#include <filesystem>
#include <memory>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include "av_log.h"
#include "av_string.h"
#include "av_time.h"
#include "av_md5.h"

#include "av_http_v2.h"

namespace fs = std::filesystem;

namespace av::http_v2 {
	// parseUrl
	static bool parseUrl(const std::string &url, std::string &scheme, std::string &host, int &port,
	                     std::string &full_path);

	bool parseUrl(const std::string &url, std::string &scheme, std::string &host, int &port,
	              std::string &full_path) {
		std::regex url_regex(R"((http|https)://([^:/\s]+)(:([0-9]+))?(/[^?\s]*)?(\?([^#]*))?)");
		std::smatch matches;

		//
		if (std::regex_match(url, matches, url_regex)) {
			// scheme (http or https)
			scheme = matches[1];

			// host
			host = matches[2];

			// port
			port = (matches[4].matched) ? std::stoi(matches[4].str()) : (scheme == "https" ? 443 : 80);

			// path 和 query parameters
			full_path = matches[5].matched ? matches[5].str() : "/";
			if (matches[6].matched) {
				full_path += "?" + matches[6].str().substr(1);
			}
			return true;
		}
		logw("url format is invalid");
		return false;
	}

	void parseCookie(const Header &header, Cookie &cookie) {
		if (header.kv.size() == 0) return;

		// lookup set-cookie
		std::string cookie_str;
		for (auto &h: header.kv) {
			if (str::toLower(h.first) == "set-cookie") {
				cookie_str = h.second;
				break;
			}
		}

		//
		if (cookie_str.empty()) {
			return;
		}

		//
		size_t start = 0;
		size_t end = 0;
		while ((end = cookie_str.find(";", start)) != std::string::npos) {
			auto cookie_part = cookie_str.substr(start, end - start);
			if (size_t cookie_pos = cookie_part.find("=", 0); cookie_pos != std::string::npos) {
				// key = value
				auto cookie_key = cookie_part.substr(0, cookie_pos);
				auto cookie_vaue = cookie_part.substr(cookie_pos + 1);
				cookie_key = av::str::trim(cookie_key);
				cookie_vaue = av::str::trim(cookie_vaue);
				cookie.kv.insert({cookie_key, cookie_vaue});
			} else {
				// only value
				cookie_part = av::str::trim(cookie_part);
				cookie.val.push_back(cookie_part);
			}
			start = end + 1;
		}
		return;
	}


	// get
	Result Client::get(const std::string &url) {
		return request(Method::Get, url, nullptr, nullptr, "");
	}

	Result Client::get(const std::string &url, const Header &header) {
		return request(Method::Get, url, &header, nullptr, "");
	}

	// post raw
	Result Client::post(const std::string &url, const std::string &body) {
		return request(Method::Post, url, nullptr, nullptr, body);
	}

	Result Client::post(const std::string &url, const Header &header, const std::string &body) {
		return request(Method::Post, url, &header, nullptr, body);
	}

	// post form
	Result Client::post(const std::string &url, const Form &form) {
		return request(Method::Post, url, nullptr, &form, "");
	}

	Result Client::post(const std::string &url, const Header &header, const Form &form) {
		return request(Method::Post, url, &header, &form, "");
	}

	Result Client::request(Method method, const std::string &url, const Header *header, const Form *form,
	                       const std::string &raw) {
		// response
		Result response = std::make_unique<Response>();

		// parse url
		std::string scheme;
		std::string host;
		int port;
		std::string full_path;
		if (!parseUrl(url, scheme, host, port, full_path)) {
			logw("parse {} failed", url);
			response->reason = fmt::format("parse url {} failed", url);
			return response;
		}
		logi("parse url {}, {}, {}, {}", scheme, host, port, full_path);
		//

		char buff[1024];
		do {
			// http
			if (scheme == "http") {
				if (port == 80) {
					snprintf(buff, sizeof(buff) - 1, "http://%s", host.c_str());
					break;
				}
				snprintf(buff, sizeof(buff) - 1, "http://%s:%d", host.c_str(), port);
				break;
			}

			// https
			if (scheme == "https" && port == 443) {
				if (port == 443) {
					snprintf(buff, sizeof(buff) - 1, "https://%s", host.c_str());
					break;
				}
				snprintf(buff, sizeof(buff) - 1, "https://%s:%d", host.c_str(), port);
				break;
			}
			logw("scheme {} not support", scheme);
			response->reason = fmt::format("scheme {} not support", scheme);
			return response;
		} while (0);

		// client
		auto scheme_host = std::string(buff);
		httplib::Client cli(scheme_host);
		if (m_timeout.has_value()) {
			cli.set_max_timeout(m_timeout.value());
		}

		// header
		httplib::Headers headers;
		headers.insert({"User-Agent", "MTeam Http Tool v2"});
		if (header != nullptr) {
			for (auto &h: header->kv) {
				headers.insert({h.first, h.second});
			}
			std::vector<std::string> coo;
			for (auto &c: header->cookie.kv) {
				coo.push_back(c.first + "=" + c.second);
			}
			for (auto &c: header->cookie.val) {
				coo.push_back(c);
			}
			if (coo.size() > 0) {
				std::string cookie_str = fmt::format("{}", fmt::join(coo, ";"));
				headers.insert({"Set-Cookie", cookie_str});
			}
		}

		httplib::Result res;
		if (method == Method::Get) {
			// get
			res = cli.Get(full_path, headers);
		} else if (method == Method::Post) {
			// post raw
			if (!raw.empty()) {
				// post raw
				res = cli.Post(full_path, headers, raw, "");
			} else if (form != nullptr) {
				// post form

				httplib::UploadFormDataItems fd;
				for (auto &f: form->kv) {
					httplib::UploadFormData d;
					d.content = f.second;
					d.name = f.first;
					fd.push_back(d);
				}
				for (auto &f: form->file) {
					httplib::UploadFormData d;
					d.name = f.first;

					// filename
					fs::path p = f.second;
					d.filename = p.filename().string();

					// read file content
					std::ifstream ifs(f.second, std::ios::binary);
					d.content = std::string((std::istreambuf_iterator<char>(ifs)),
					                        std::istreambuf_iterator<char>());

					fd.push_back(d);
				}
				std::string boundary_tmp = fmt::format("boundary{}", av::time::microseconds());
				std::string boundary;
				if (!av::hash::md5(boundary_tmp, boundary)) {
					boundary = boundary_tmp;
				}
				res = cli.Post(full_path, headers, fd, boundary);
			}
		}

		// check res
		if (!res) {
			logw("http request failed");
			response->reason = "http request failed";
			return response;
		}

		// response data
		response->status = res->status;
		response->body = res->body;
		response->location = res->location;
		response->reason = res->reason;
		for (const auto &h: res->headers) {
			response->header.kv[h.first] = h.second;
		}
		return response;
	}
}

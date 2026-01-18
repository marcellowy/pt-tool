#include <optional>
#include <curl/curl.h>

#include "av_log.h"
#include "av_async.h"
#include "av_string.h"

#include "av_http_v3.h"

namespace av::http_v3 {
	size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
	size_t headerCallback(void* ptr, size_t size, size_t nmemb, std::string* data);
	static void parseHeader(const std::string& header_str, Header& header);

	bool parseCookie(const Header& header, Cookie& cookie) {
		if (header.kv.size() == 0) return true;

		// lookup set-cookie
		std::string cookie_str;
		for (auto& h : header.kv) {
			if (str::toLower(h.first) == "set-cookie") {
				cookie_str = h.second;
				break;
			}
		}

		//
		if (cookie_str.empty()) {
			return true;
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
				cookie.kv.insert({ cookie_key , cookie_vaue});
			}
			else {
				// only value
				cookie_part = av::str::trim(cookie_part);
				cookie.val.push_back(cookie_part);
			}
			start = end + 1;
		}
		return true;
	}

    Client::Client() {

    }

    Client::~Client(){

    }

    // get
	Result Client::get(const std::string& url) {
		return request(Method::Get, url, nullptr, nullptr, "");
	}
	Result Client::get(const std::string& url, const Header& header) {
		return request(Method::Get, url, &header, nullptr, "");
	}

	// post raw
	Result Client::post(const std::string& url, const std::string& body) {
		return request(Method::Post, url, nullptr, nullptr, body);
	}
	Result Client::post(const std::string& url, const Header& header, const std::string& body) {
		return request(Method::Post, url, &header, nullptr, body);
	}

	// post form
	Result Client::post(const std::string& url, const Form& form) {
		return request(Method::Post, url, nullptr, &form, "");
	}

	Result Client::post(const std::string& url, const Header& header, const Form& form) {
		return request(Method::Post, url, &header, &form, "");
	}

	Result Client::request(Method method, const std::string& url, const Header* header, const Form* form, const std::string& raw) {
	    // response
    	Result response = std::make_unique<Response>();
    	CURL* curl = curl_easy_init();
		if (!curl) {
			logw("curl_easy_init failed");
			response->reason = "Failed to initialize curl";
			return response;
		}
		av::async::Exit exit_curl([&curl] {
			curl_easy_cleanup(curl);
		});

    	// set url
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		bool userAgent = false;
    	// header
	    curl_slist* hds = nullptr;
		for (const auto& h : header->kv) {
			std::stringstream s;
			s << h.first << ": " << h.second;
			logi("add header {}", s.str());
			auto key_lower = av::str::toLower(h.first);
			if (key_lower == "user-agent") {
				userAgent = true;
			}
			hds = curl_slist_append(hds, s.str().c_str());
		}
		if (!userAgent) {
			std::stringstream s;
			s << "User-Agent: " << "MTeam Http Tool v3";
			logi("add header {}", s.str());
			hds = curl_slist_append(hds, s.str().c_str());
		}
    	if (hds != nullptr) {
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hds);
		}
		av::async::Exit exit_header([&hds] {
			if(hds != nullptr) curl_slist_free_all(hds);
		});

		// form mime
		curl_mime* mime = curl_mime_init(curl);
		if (mime == nullptr) {
			loge("init mime failed");
			response->reason = "Failed to initialize curl mime";
			return response;
		}
		av::async::Exit exit_mime([&mime] {
			curl_mime_free(mime);
		});

    	if (method == Method::Post) {
    		do {
    			if (!raw.empty()) {
    				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, raw.c_str());
    				break;
    			}
				curl_mimepart* part = NULL;

				// add form data
				for (const auto& data : form->kv) {
					part = curl_mime_addpart(mime);
					curl_mime_name(part, data.first.c_str());
					curl_mime_data(part, data.second.c_str(), CURL_ZERO_TERMINATED);
					logi("add form {} = {}", data.first, data.second);
				}

				// add file
				for (const auto& f : form->file) {
					part = curl_mime_addpart(mime);
					curl_mime_name(part, f.first.c_str());
					curl_mime_filedata(part, f.second.c_str());
					logi("add file {} = {}", f.first, f.second);
				}

				curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    		} while (false);
    	}

		// Optionally, if you want to follow redirects (e.g., HTTP 3xx responses)
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

		// write callback
		std::string response_string;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

		// response header
		std::string response_header;
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);

		// Set a timeout in seconds (e.g., 10 seconds)
    	if (m_timeout.has_value()) {
    		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(m_timeout.value()));  // Timeout in seconds
    	}
    	CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			logi("http send not ok");
			response->reason = "Failed to perform curl";
			return response;
		}

    	// code
		long response_code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    	response->status = static_cast<int>(response_code);
    	response->body = response_string;

    	// parse header
    	parseHeader(response_header, response->header);

    	// response->header
    	if ( (response->status == 301 || response->status == 302) &&
    		response->header.kv.contains("Location")) {
    		response->location = response->header.kv["Location"];
    	}
    	return response;
    }

	size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		static_cast<std::string *>(userp)->append(static_cast<char *>(contents), size * nmemb);
		return size * nmemb;
	}

	size_t headerCallback(void* ptr, size_t size, size_t nmemb, std::string* data) {
		size_t totalSize = size * nmemb;
		data->append(static_cast<char*>(ptr), totalSize);
		return totalSize;
	}

	void parseHeader(const std::string& header_str,  Header& header) {
		size_t start = 0;
		size_t end = 0;
		while ((end = header_str.find("\r\n", start)) != std::string::npos) {
			auto line = header_str.substr(start, end - start);
			if (size_t pos = line.find(":", 0); pos != std::string::npos) {
				auto key = line.substr(0, pos);
				auto value = line.substr(pos + 1);
				key = av::str::trim(key);
				value = av::str::trim(value);
				header.kv[key] = value;
			}
			start = end + 2;
		}
	}
}
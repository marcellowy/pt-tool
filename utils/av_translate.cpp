#include "av_translate.h"

#include "nlohmann/json.hpp"
#include <curl/curl.h>

#include "av_log.h"
#include "av_async.h"

using json = nlohmann::json;

namespace av {
	namespace translate {

		size_t CURLWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
			((std::string*)userp)->append((char*)contents, size * nmemb);
			return size * nmemb;
		}

		Translate::Translate(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host) :
			m_rapidapi_key(rapidapi_key),
			m_rapidapi_host(rapidapi_host)
		{

		}

		bool Translate::foo(const std::tstring& source_text, std::tstring& text)
		{
			json a;
			a["from"] = "auto";
			a["to"] = "en";
			a["text"] = av::str::toA(source_text);

			// dump
			auto data = a.dump();
			logi("post json: {}", data);

			// curl post
			CURL* curl;
			CURLcode res;
			curl = curl_easy_init();
			if (!curl) {
				loge("curl_easy_init failed");
				return false;
			}
			av::async::Exit exit_curl([&curl] {
				curl_easy_cleanup(curl);
			});
			
			// 
			curl_easy_setopt(curl, CURLOPT_URL, av::str::toA(m_rapidapi_url).c_str());

			// header
			struct curl_slist* headers = NULL;
			std::string header_key = "x-rapidapi-key: " + av::str::toA(m_rapidapi_key);
			std::string header_host = "x-rapidapi-host: " + av::str::toA(m_rapidapi_host);
			headers = curl_slist_append(headers, "Content-Type: application/json");
			headers = curl_slist_append(headers, header_key.c_str());
			headers = curl_slist_append(headers, header_host.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			av::async::Exit exit_header([&headers] {
				curl_slist_free_all(headers);
			});

			// post data
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

			// response callback
			std::string response_string;
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

			// 
			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				loge("curl_easy_perform failed, err: {}", curl_easy_strerror(res));
				return false;
			}

			// code
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			if (response_code != 200) {
				loge("http response status code {}, body {}", response_code, response_string);
				return false;
			}
			
			// parse response body
			try {
				json b;
				auto o = b.parse(response_string);
				if (o.contains("trans") && o["trans"].is_string())
				{
					text = av::str::toT(o["trans"].get<std::string>());
					return true;
				}
				logw("no trans field");
				return false;
			}
			catch (const json::parse_error& e) {
				loge("{} exception {}", response_string, e.what());
				return false;
			}
			catch (const std::exception& e) {
				loge("{} exception {}", response_string, e.what());
				return false;
			}
			return false;
		}
	}
}
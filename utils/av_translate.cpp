#include "av_translate.h"

#include "nlohmann/json.hpp"
#include <curl/curl.h>

#include "av_log.h"
#include "av_async.h"
#include "av_http.h"

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
			auto data = a.dump(4);
			logi("post json: {}", data);

			av::http::Client client;
			av::http::Header header;
			header.data[TEXT("x-rapidapi-key")] = m_rapidapi_key;
			header.data[TEXT("x-rapidapi-host")] = m_rapidapi_host;
			header.data[TEXT("User-Agent")] = TEXT("team tptv");
			header.data[TEXT("Content-Type")] = TEXT("application/json");
			av::http::Response resp;
			client.setConnectTimeoutMS(10000);
			client.setTimeoutMS(30000);
			std::tstring tmp = av::str::toT(data);
			if (!client.post(m_rapidapi_url, header, tmp, resp)) {
				loge("post data failed");
				return false;
			}

			// parse response body
			try {
				json b;
				auto o = b.parse(av::str::toA(resp.body));
				if (o.contains("trans") && o["trans"].is_string())
				{
					text = av::str::toT(o["trans"].get<std::string>());
					return true;
				}
				logw("no trans field");
				return false;
			}
			catch (const json::parse_error& e) {
				loge("{} exception {}", av::str::toA(resp.body), e.what());
				return false;
			}
			catch (const std::exception& e) {
				loge("{} exception {}", av::str::toA(resp.body), e.what());
				return false;
			}
			return false;
		}
	}
}

#include "av_translate.h"

#include "nlohmann/json.hpp"
#include <curl/curl.h>

#include "av_log.h"
#include "av_async.h"
#include "av_http.h"

using json = nlohmann::json;
namespace http_ = av::http;

namespace av {
	namespace translate {
		Translate::Translate(const std::tstring &rapidapi_key,
		                     const std::tstring &rapidapi_host) : m_rapidapi_key(rapidapi_key),
		                                                          m_rapidapi_host(rapidapi_host) {
		}

		bool Translate::foo(const std::tstring &source_text, std::tstring &text) {
			if (source_text.empty()) {
				logw("source text empty");
				return false;
			}
			json a;
			a["from"] = "auto";
			a["to"] = "en";
			a["text"] = av::str::toA(source_text);

			// dump
			auto data = a.dump(4);
			logi("post json: {}", data);

			http_::Client client;
			http_::Header header;
			header.kv["x-rapidapi-key"] = av::str::toA(m_rapidapi_key);
			header.kv["x-rapidapi-host"] = av::str::toA(m_rapidapi_host);
			header.kv["User-Agent"] = "team tptv";
			header.kv["Content-Type"] = "application/json";

			const auto resp = client.post(av::str::toA(m_rapidapi_url), header, data);
			if (!resp) {
				loge("post data failed");
				return false;
			}
			logi("translate json: {}", resp->body);

			// parse response body
			try {
				const nlohmann::json o = json::parse(resp->body);
				if (o.contains("trans") && o["trans"].is_string()) {
					text = av::str::toT(o["trans"].get<std::string>());
					if (text.empty()) return false;
					return true;
				}
				logw("no trans field");
				return false;
			} catch (const json::parse_error &e) {
				loge("{} exception {}", resp->body, e.what());
				return false;
			} catch (const std::exception &e) {
				loge("{} exception {}", resp->body, e.what());
				return false;
			}
			return false;
		}
	}
}

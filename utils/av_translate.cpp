#include "av_translate.h"

#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include <unordered_map>

#include "av_log.h"
#include "av_async.h"
#include "av_http.h"

using json = nlohmann::json;
namespace http_ = av::http;

namespace av {
	namespace translate {

		std::unique_ptr<ITranslate> TranslateFactory::create(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host, TranslateType type) {
			switch (type) {
			case TranslateType::kFreeGoogleTranslate:
				return std::make_unique<FreeGoogleTranslate>(rapidapi_key, rapidapi_host);
			case TranslateType::kTranslate:
				return std::make_unique<Translate>(rapidapi_key, rapidapi_host);
			default:
				logw("unknown translate type");
				return nullptr;
			}
		}

		FreeGoogleTranslate::FreeGoogleTranslate(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host) :
			m_rapidapi_key(rapidapi_key),
			m_rapidapi_host(rapidapi_host) {
		}

		bool FreeGoogleTranslate::foo(const std::tstring& source_text, std::tstring& text) {
			if (source_text.empty()) {
				logw("source text empty");
				return false;
			}
			json a;
			a["translate"] = "rapidapi";

			// dump
			auto data = a.dump(4);

			//
			char url[2048] = { 0 };
			snprintf(url, 2048, av::str::toA(m_rapidapi_url).c_str(), av::str::toA(source_text).c_str());

			http_::Client client;
			http_::Header header;
			header.kv["x-rapidapi-key"] = av::str::toA(m_rapidapi_key);
			header.kv["x-rapidapi-host"] = av::str::toA(m_rapidapi_host);
			header.kv["User-Agent"] = "team tptv";
			header.kv["Content-Type"] = "application/json";

			logi("post json: {}, post url: {}", data, url);
			const auto resp = client.post(url, header, data);
			if (!resp) {
				loge("post data failed");
				return false;
			}
			logi("translate json: {}", resp->body);

			// parse response body
			try {
				const nlohmann::json o = json::parse(resp->body);
				if (!o.contains("status") || !o["status"].is_number_integer() || o["status"].get<int>() != 200 || !o.contains("translation") || !o["translation"].is_string()) {
					logw("no translation field or status not 200");
					return false;
				}

				//
				text = av::str::toT(o["translation"].get<std::string>());
				if (text.empty()) return false;
				return true;
			}
			catch (const json::parse_error& e) {
				loge("{} exception {}", resp->body, e.what());
				return false;
			}
			catch (const std::exception& e) {
				loge("{} exception {}", resp->body, e.what());
				return false;
			}
			return false;
		}

		Translate::Translate(const std::tstring& rapidapi_key,
			const std::tstring& rapidapi_host) : m_rapidapi_key(rapidapi_key),
			m_rapidapi_host(rapidapi_host) {
		}

		bool Translate::foo(const std::tstring& source_text, std::tstring& text) {
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
			}
			catch (const json::parse_error& e) {
				loge("{} exception {}", resp->body, e.what());
				return false;
			}
			catch (const std::exception& e) {
				loge("{} exception {}", resp->body, e.what());
				return false;
			}
			return false;
		}
	}
}

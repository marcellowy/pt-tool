#include "av_translate.h"

#include "nlohmann/json.hpp"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "httplib/httplib.h"

#include "av_log.h"

using json = nlohmann::json;

namespace av {
	namespace translate {
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

			auto b = a.dump();
			logi("json: {}", b);

			httplib::Client client(av::str::toA(m_rapidapi_url));
			httplib::Headers headers = {
				{ "Content-Type", "application/json" },
				{ "x-rapidapi-key", av::str::toA(m_rapidapi_key)},
				{ "x-rapidapi-host", av::str::toA(m_rapidapi_host)},
			};
			client.set_max_timeout(3000);
			auto res = client.Post("/api/v1/translator/text", headers, b, "application/json");
			if (res && res->status == 200) {
				json b;
				auto body = res->body;
				logi("translate response body {}", body);
				try{
					auto o = b.parse(res->body);
					if (o.contains("trans") && o["trans"].is_string())
					{
						text = av::str::toT(o["trans"].get<std::string>());
						return true;
					}
					logw("no trans field");
					return false;
				}
				catch (const json::parse_error& e) {
					loge("{} exception {}", body, e.what());
					return false;
				}
				catch (const std::exception& e) {
					loge("{} exception {}", body, e.what());
					return false;
				}
			}
			loge("response code {}, body {}", res->status, res->body);
			return false;
		}
	}
}
#include "av_qbittorrent.h"

#include <map>
#include <istream>
#include <curl/curl.h>
#include <tuple>

#include "av_async.h"
#include "av_http.h"
#include "av_log.h"

namespace http_ = av::http;

namespace av {
	namespace qbittorrent {
		Qbittorrent::Qbittorrent(const std::tstring& api_url, const std::tstring& username, const std::tstring& password):
			m_api_url (api_url), 
			m_username (username), 
			m_password (password)
		{

		}

		bool Qbittorrent::login() {
			// url
			std::tstring url = m_api_url;
			url.append(TEXT("/api/v2/auth/login"));

			http_::Client client;
			http_::Form form;
			form.kv["username"] = str::toA(m_username);
			form.kv["password"] = str::toA(m_password);
			const auto resp = client.post(av::str::toA(url), form);
			if (!resp) {
				loge("http request error");
				return false;
			}
			if (resp->status == 200) {
				logi("is ok");
				for (auto& aa : resp->header.kv) {
					logi("header {}: {}", aa.first, aa.second);
				}
			}
			return false;
		}
	}
}
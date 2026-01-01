#include "av_qbittorrent.h"

#include <map>
#include <istream>
#include <curl/curl.h>
#include <tuple>

#include "av_async.h"
#include "av_http.h"
#include "av_log.h"

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

			av::http::Client client;
			av::http::FormData d;
			d.data[TEXT("username")] = m_username;
			d.data[TEXT("password")] = m_password;
			av::http::Response res;
			if (!client.postForm(url, d, res)) {
				loge("http request error");
				return false;
			}
			if (res.isOk()) {
				logi("is ok");
				for (auto& aa : res.header.data) {
					logi("header {}: {}", av::str::toA(aa.first), av::str::toA(aa.second));
				}
			}
			return false;
		}
	}
}
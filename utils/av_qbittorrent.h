#ifndef AV_QBITTORRENT_H_
#define AV_QBITTORRENT_H_

#include "av_string.h"

namespace av {
	namespace qbittorrent {
	
		class Qbittorrent {
		public:
			Qbittorrent() = default;
			Qbittorrent(const std::tstring& api_url, const std::tstring& username, const std::tstring& password);
			~Qbittorrent() = default;
			bool login();
		private:
			bool post(const std::tstring& url, const std::map<std::tstring, std::tstring>& body);
		private:
			std::tstring m_api_url;
			std::tstring m_username;
			std::tstring m_password;
		};

	}
}

#endif
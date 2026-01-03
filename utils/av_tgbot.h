#ifndef AV_TIME_H_
#define AV_TIME_H_

#include <string>

namespace av {
	namespace tgbot {
		bool send_message(const std::string& token, const std::string& chat_id, const std::string& text);
		bool send_local_photo_message(const std::string& token, const std::string& chat_id, const std::string& local_img, const std::string& text);
		bool send_net_photo_message(const std::string& token, const std::string& chat_id, const std::string& net_img, const std::string& text);
	}
}

#endif

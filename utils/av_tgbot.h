#ifndef AV_TIME_H_
#define AV_TIME_H_

#include <string>

namespace av {
	namespace tgbot {
		bool send_message(const std::string& token, const std::string& chat_id, const std::string& text);
		bool send_message(const std::string& token, int64_t chat_id, const std::string& text);
	}
}

#endif

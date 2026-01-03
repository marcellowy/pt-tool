#include "av_tgbot.h"

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00  // 设置最低支持 Win10
#endif
#endif
#include "tgbot/tgbot.h"

#include "av_log.h"

namespace av {
	namespace tgbot {
		bool send_message(const std::string& token, const std::string& chat_id, const std::string& text) {
			TgBot::Bot bot(token);
			try {
				bot.getApi().sendMessage(chat_id, text);
			}
			catch (TgBot::TgException& e) {
				loge("{}", e.what());
				return false;
			}
			return true;
		}


		bool send_local_photo_message(const std::string& token, const std::string& chat_id, const std::string& local_img, const std::string& text) {
			TgBot::Bot bot(token);
			try {
				auto msg_ptr = bot.getApi().sendPhoto(chat_id, TgBot::InputFile::fromFile(local_img, "image/png"), text);
				logi("send message success {}", msg_ptr->messageId);
			}
			catch (TgBot::TgException& e) {
				loge("{}", e.what());
				return false;
			}
			return true;
		}

		bool send_net_photo_message(const std::string& token, const std::string& chat_id, const std::string& net_img, const std::string& text) {
			TgBot::Bot bot(token);
			try {
				auto msg_ptr = bot.getApi().sendPhoto(chat_id, net_img, text);
				logi("send message success {}", msg_ptr->messageId);
			}
			catch (TgBot::TgException& e) {
				loge("{}", e.what());
				return false;
			}
			return true;
		}
	}
}

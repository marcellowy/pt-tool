#include "av_tgbot.h"

#include "av_http.h"
#include "av_http_v3.h"
#include "av_log.h"

namespace http_ = av::http_v3;

namespace av {
	namespace tgbot {
		bool send_message(const std::string& token, const std::string& chat_id, const std::string& text) {
			http_::Client client;
			http_::Form form;
			form.kv["chat_id"] = chat_id;
			form.kv["text"] = text;

			// url buff
			char buff[1024];
			snprintf(buff, sizeof(buff) - 1, "https://api.telegram.org/bot%s/sendMessage", av::str::toA(token).c_str());

			// http
			const auto resp = client.post(std::string(buff), form);
			if (!resp) {
				logw("post form failed");
				return false;
			}
			if ( resp->status == 200) return true;
			logw("{}, {}", resp->status, resp->body);
			return false;
		}

		bool send_local_photo_message(const std::string& token, const std::string& chat_id, const std::string& local_img, const std::string& text) {
			http_::Client client;
			http_::Form form;
			form.kv["chat_id"] = chat_id;
			form.kv["caption"] = text;
			form.file["photo"] = local_img;

			// url buff
			char buff[1024];
			snprintf(buff, sizeof(buff) - 1, "https://api.telegram.org/bot%s/sendPhoto", av::str::toA(token).c_str());

			// http
			const auto resp = client.post(std::string(buff), form);
			if (!resp) {
				logw("post form failed");
				return false;
			}
			if (resp->status == 200) return true;
			logw("{}, {}", resp->status, resp->body);
			return false;
		}

		bool send_net_photo_message(const std::string& token, const std::string& chat_id, const std::string& net_img, const std::string& text) {
			
			http_::Client client;
			http_::Form form;
			form.kv["chat_id"] = chat_id;
			form.kv["photo"] = net_img;
			form.kv["caption"] = text;

			// url buff
			char buff[1024];
			snprintf(buff, sizeof(buff) - 1, "https://api.telegram.org/bot%s/sendPhoto", av::str::toA(token).c_str());

			// http
			const auto resp = client.post(std::string(buff), form);
			if (!resp) {
				logw("post form failed");
				return false;
			}
			if (resp->status == 200) return true;
			logw("{}, {}", resp->status, resp->body);
			return false;
		}
	}
}

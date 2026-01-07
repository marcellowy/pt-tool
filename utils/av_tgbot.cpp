#include "av_tgbot.h"

#include "av_http.h"
#include "av_log.h"

namespace av {
	namespace tgbot {
		bool send_message(const std::string& token, const std::string& chat_id, const std::string& text) {
			av::http::Client client;
			av::http::Form form;
			form.data[TEXT("chat_id")] = av::str::toT(chat_id);
			form.data[TEXT("text")] = av::str::toT(text);

			// response
			av::http::Response resp;

			// url buff
			char buff[1024];
			snprintf(buff, sizeof(buff) - 1, "https://api.telegram.org/bot%s/sendMessage", av::str::toA(token).c_str());

			// http
			if (!client.postForm(av::str::toT(std::string(buff)), form, resp)) {
				logw("post form failed");
				return false;
			}
			if (resp.isOk()) return true;

			logw("{}, {}", resp.code, av::str::toA(resp.body));
			return false;
		}

		bool send_local_photo_message(const std::string& token, const std::string& chat_id, const std::string& local_img, const std::string& text) {
			av::http::Client client;
			client.setRetryTimes(2); // retry 2 times
			av::http::Form form;
			form.data[TEXT("chat_id")] = av::str::toT(chat_id);
			form.data[TEXT("caption")] = av::str::toT(text);

			av::http::Header header;
			av::http::File file;
			file.data[TEXT("photo")] = av::str::toT(local_img);
			
			// response
			av::http::Response resp;

			// url buff
			char buff[1024];
			snprintf(buff, sizeof(buff) - 1, "https://api.telegram.org/bot%s/sendPhoto", av::str::toA(token).c_str());

			// http
			if (!client.postForm(av::str::toT(std::string(buff)), std::make_tuple(form, file), resp)) {
				logw("post form failed");
				return false;
			}
			if (resp.isOk()) return true;

			logw("{}, {}", resp.code, av::str::toA(resp.body));
			return false;
		}

		bool send_net_photo_message(const std::string& token, const std::string& chat_id, const std::string& net_img, const std::string& text) {
			
			av::http::Client client;
			av::http::Form form;
			form.data[TEXT("chat_id")] = av::str::toT(chat_id);
			form.data[TEXT("photo")] = av::str::toT(net_img);
			form.data[TEXT("caption")] = av::str::toT(text);

			// resposne
			av::http::Response resp;

			// url buff
			char buff[1024];
			snprintf(buff, sizeof(buff) - 1, "https://api.telegram.org/bot%s/sendPhoto", av::str::toA(token).c_str());

			// http
			if (!client.postForm(av::str::toT(std::string(buff)), form, resp)) {
				logw("post form failed");
				return false;
			}
			if (resp.isOk()) return true;

			logw("{}, {}", resp.code, av::str::toA(resp.body));
			return false;
		}
	}
}

#ifndef AV_HTTP_V2_H_
#define AV_HTTP_V2_H_

#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <optional>
#include <vector>

namespace av {
	namespace http_v2 {
		enum class Method {
			Get,
			Post
		};

		struct Form {
			std::map<std::string, std::string> kv;
			std::map<std::string, std::string> file;
		};

		struct Cookie {
			std::map<std::string, std::string> kv;
			std::vector<std::string> val;
		};

		struct Header {
			std::map<std::string, std::string> kv;
			Cookie cookie;
		};

		struct Response {
			int status = -1;
			std::string reason;
			Header header;
			std::string body;
			std::string location;
		};

		typedef std::unique_ptr<Response> Result;

		// cookie
		static bool parseCookie(const Header& header, Cookie& cookie);

		// http client
		class Client
		{
		public:
			Client();
			~Client();

			template <class Rep, class Period>
			void setTimeout(std::chrono::duration<Rep, Period> duration) {
				m_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
			}

			// get
			Result get(const std::string& url);
			Result get(const std::string& url, const Header& header);
			// post raw
			Result post(const std::string& url, const std::string& body);
			Result post(const std::string& url, const Header& header, const std::string& body);
			// post form
			Result post(const std::string& url, const Form& form);
			Result post(const std::string& url, const Header& header, const Form& form);
		private:
			Result request(Method method, const std::string& url, const Header* header, const Form* form, const std::string& raw);
			std::optional<int64_t> m_timeout;
		};
	}
}

#endif

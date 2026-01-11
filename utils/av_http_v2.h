#ifndef AV_HTTP_V2_H_
#define AV_HTTP_V2_H_

#include <map>
#include <string>

namespace {
	enum class Method {
		Get,
		Post
	};
}

namespace av {
	namespace http_v2 {

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
			int status;
			Header header;
			std::string body;
			std::string location;
		};

		// cookie
		bool parseCookie(const Header& header, Cookie& cookie);

		// http client
		class Client
		{
		public:
			Client();
			~Client();
		public:
			// get
			bool get(const std::string& url, Response& response);
			bool get(const std::string& url, const Header& header, Response& response);
			// post raw
			bool post(const std::string& url, const std::string& body, Response& response);
			bool post(const std::string& url, const Header& header, const std::string& body, Response& response);
			// post form
			bool post(const std::string& url, const Form& form, Response& response);
			bool post(const std::string& url, const Header& header, const Form& form, Response& response);
		private:
			bool request(Method method, const std::string& url, const Header* header, const Form* form, const std::string& raw, Response& response);
		};
	}
}

#endif

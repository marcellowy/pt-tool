#ifndef AV_HTTP_V2_H_
#define AV_HTTP_V2_H_

#include <map>
#include <string>
#include <memory>

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
			int status = -1;
			std::string reason = "";
			Header header;
			std::string body = "";
			std::string location = "";
		};

		typedef std::unique_ptr<Response> Result;

		// http client
		class Client
		{
		public:
			Client();
			~Client();
		public:
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
		};
	}
}

#endif

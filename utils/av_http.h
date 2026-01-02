#ifndef AV_HTTP_H_
#define AV_HTTP_H_

#include "curl/curl.h"
#include <map>
#include <vector>
#include <utility>
#include <tuple>
#include <variant>
#include "av_string.h"

namespace av {
	namespace http {

		struct Cookie {
			std::map<std::tstring, std::tstring> data;
		};

		struct Header {
			std::map<std::tstring, std::tstring> data;
			Cookie cookie;
		};

		struct FormData {
			std::map<std::tstring, std::tstring> data;
		};

		struct File {
			std::map<std::tstring, std::tstring> data;
		};

		enum class Method {
			Get,
			Post
		};

		class Response {
		public:
			int64_t code = 0;
			Header header;
			std::tstring body;
		public:
			Response() = default;
			~Response() = default;
			bool isOk();
		};

		class Client {
		private:
			using paramType = std::variant<
				Header,
				FormData,
				File, 
				std::tuple<Header, FormData>, 
				std::tuple<Header, File>,
				std::tuple<FormData, File>,
				std::tuple<Header, FormData, File>
			>;
			enum class RequestBodyType {
				Normal,
				Form
			};

			struct RequestBody {
				RequestBodyType type;
				std::tstring body;			// type = Normal
				std::map<std::tstring, std::tstring> form;
				std::map<std::tstring, std::tstring> file;
			};
		public:
			Client() = default;
			~Client() = default;
			bool get(const std::tstring& url, Response& response);
			bool get(const std::tstring& url, const Header& header, Response& response);
			bool post(const std::tstring& url, const Header& header, const std::tstring& body, Response& response);
			bool post(const std::tstring& url, const std::tstring& body, Response& response);
			bool postForm(const std::tstring& url, const paramType& param, Response& response);
		public:
			void setTimeoutMS(int64_t timeout);
			void setConnectTimeoutMS(int64_t timeout);
		private:
			bool request(const Method& method, const std::tstring& url, const Header& header, const RequestBody& request, Response& response);
			void parseHeader(const std::string& header_str, Response& response);
		private:
			bool m_need_response_headers = false;
			int64_t m_timeout_ms = 10000;
			int64_t m_connect_timeout_ms = 5000;
		};
	}
}

#endif

#include "av_http.h"
#include "av_log.h"
#include "av_async.h"
#include <tuple>
#include <variant>

namespace av {
	namespace http {

		bool Response::isOk() {
			return code == 200;
		}

		size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

		size_t headerCallback(void* ptr, size_t size, size_t nmemb, std::string* data);

		void Client::setTimeoutMS(int64_t timeout) {
			m_timeout_ms = timeout;
		}
		
		void Client::setConnectTimeoutMS(int64_t timeout) {
			m_connect_timeout_ms = timeout;
		}

		bool Client::get(const std::tstring& url, Response& response) {
			Header h{};
			RequestBody rb;
			return request(Method::Get, url, h, rb, response);
		}

		bool Client::get(const std::tstring& url, const Header& header, Response& response) {
			RequestBody rb;
			return request(Method::Get, url, header, rb, response);
		}

		bool Client::post(const std::tstring& url, const Header& header, const std::tstring& body, Response& response) {
			RequestBody rb;
			rb.type = RequestBodyType::Normal;
			rb.body = body;
			return request(Method::Post, url, header, rb, response);
		}

		bool Client::post(const std::tstring& url, const std::tstring& body, Response& response) {
			Header h{};
			RequestBody rb;
			rb.type = RequestBodyType::Normal;
			rb.body = body;
			return request(Method::Post, url, h, rb, response);
		}

		bool Client::postForm(const std::tstring& url, const paramType& param, Response& response) {
			Header header;
			FormData form;
			File file;
			std::visit([&header, &form, &file](auto&& arg) {
				if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, Header>) {
					header = arg;
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, FormData>) {
					form = arg;
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, File>) {
					file = arg;
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::tuple<Header, FormData>>) {
					header = std::get<0>(arg);
					form = std::get<1>(arg);
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::tuple<Header, File>>) {
					header = std::get<0>(arg);
					file = std::get<1>(arg);
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::tuple<FormData, File>>) {
					form = std::get<0>(arg);
					file = std::get<1>(arg);
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::tuple<Header, FormData, File>>) {
					header = std::get<0>(arg);
					form = std::get<1>(arg);
					file = std::get<2>(arg);
				}
			}, param);

			// request struct
			RequestBody rb;
			rb.type = RequestBodyType::Form;

			// add File
			rb.file = file.data;
			
			// add form data
			rb.form = form.data;

			return request(Method::Post, url, header, rb, response);
		}

		bool Client::request(const Method& method, const std::tstring& url, const Header& header, const RequestBody& request, Response& response) {

			response.code = 0;
			response.body = TEXT("");
			logi("libcurl version: {}", curl_version());
			CURL* curl = curl_easy_init();
			if (!curl) {
				logw("curl_easy_init failed");
				return false;
			}
			av::async::Exit exit_curl([&curl] {
				curl_easy_cleanup(curl);
			});

			// set url
			curl_easy_setopt(curl, CURLOPT_URL, av::str::toA(url).c_str());

			// header
			struct curl_slist* hds = NULL;
			for (auto& h : header.data) {
				std::tstringstream s;
				s << h.first << ": " << h.second;
				logi("add header {}", av::str::toA(s.str()));
				hds = curl_slist_append(hds, av::str::toA(s.str()).c_str());
			}
			if (hds != NULL) {
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hds);
			}
			av::async::Exit exit_header([&hds] {
				if(hds != NULL) curl_slist_free_all(hds);
			});

			// form mime
			curl_mime* mime = curl_mime_init(curl);
			if (mime == NULL) {
				loge("init mime faild");
				return false;
			}
			av::async::Exit exit_mime([&mime] {
				curl_mime_free(mime);
			});

			// set method
			switch (method) {
			case Method::Get:
			case Method::Post:
				if (request.type == RequestBodyType::Normal)
				{
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, av::str::toA(request.body).c_str());
				}
				else if (request.type == RequestBodyType::Form) {
					curl_mimepart* part = NULL;
					// add form data
					for (auto data : request.form) {
						part = curl_mime_addpart(mime);
						curl_mime_name(part, av::str::toA(data.first).c_str() );
						curl_mime_data(part, av::str::toA(data.second).c_str(), CURL_ZERO_TERMINATED);
					}

					// add file
					for (auto f : request.file) {
						part = curl_mime_addpart(mime);
						curl_mime_name(part, av::str::toA(f.first).c_str() );
						curl_mime_filedata(part, av::str::toA(f.second).c_str() );
					}

					curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
				}
			}

			// Optionally, if you want to follow redirects (e.g., HTTP 3xx responses)
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

			// write callback
			std::string response_string;
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

			// response header
			std::string response_header;
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_header);
			
			// Set a timeout in seconds (e.g., 10 seconds)
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(m_timeout_ms));  // Timeout in seconds

			// Optionally, you can set the connection timeout as well (e.g., 5 seconds)
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(m_connect_timeout_ms));  // Timeout for connection phase (seconds)

			// 
			CURLcode res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				loge("curl_easy_perform failed, err: {}", curl_easy_strerror(res));
				return false;
			}

			// code
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

			// parse response header
			{
				logi("response code {} resonse header {}", response_code, response_header);
				parseHeader(response_header, response.header);
			}

			// set response
			response.code = static_cast<int64_t>(response_code);
			response.body = av::str::toT(response_string);
			return true;
		}
		
		void Client::parseHeader(const std::string& header_str, Header& header) {
			size_t start = 0;
			size_t end = 0;

			while ((end = header_str.find("\r\n", start)) != std::string::npos) {
				auto line = header_str.substr(start, end - start);
				if (size_t pos = line.find(":", 0); pos != std::string::npos) {
					auto key = line.substr(0, pos);
					auto value = line.substr(pos + 1);
					key = av::str::trim(key);
					value = av::str::trim(value);
					header.data[av::str::toT(key)] = av::str::toT(value);
					if (av::str::toLower(key) == "set-cookie") {
						auto cookie_str = value;
						// parse cookie
						size_t cookie_start = 0;
						size_t cookie_end = 0;
						while ((cookie_end = cookie_str.find(";", cookie_start)) != std::string::npos) {
							auto cookie_part = cookie_str.substr(cookie_start, cookie_end - cookie_start);
							if (size_t cookie_pos = cookie_part.find("=", 0); cookie_pos != std::string::npos) {
								// key = value
								auto cookie_key = cookie_part.substr(0, cookie_pos);
								auto cookie_vaue = cookie_part.substr(cookie_pos + 1);
								cookie_key = av::str::trim(cookie_key);
								cookie_vaue = av::str::trim(cookie_vaue);
								header.cookie.data[av::str::toT(cookie_key)] = av::str::toT(cookie_vaue);
							}
							else {
								// only value
								cookie_part = av::str::trim(cookie_part);
								header.cookie.data[av::str::toT(cookie_part)] = av::str::toT(cookie_part);
							}
							cookie_start = cookie_end + 1;
						}
					}
				}
				start = end + 2;
			}
		}

		size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
			((std::string*)userp)->append((char*)contents, size * nmemb);
			return size * nmemb;
		}

		size_t headerCallback(void* ptr, size_t size, size_t nmemb, std::string* data) {
			size_t totalSize = size * nmemb;
			data->append(static_cast<char*>(ptr), totalSize);
			return totalSize;
		}
	}
}

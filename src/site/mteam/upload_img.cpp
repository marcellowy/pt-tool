#include <tuple>
#include "upload_img.h"
#include "av_http.h"
#include "av_log.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace mteam {
	UploadImg::UploadImg(const std::tstring& url, const std::tstring& key) : 
		m_api_img_key(key),
		m_api_img_url(url)
	{
	}

	UploadImg::~UploadImg()
	{
	}


	bool UploadImg::Upload(const std::tstring& img_path, std::tstring& url) {

		// client
		av::http::Client client;

		// response
		av::http::Response resp;

		// header
		av::http::Header header;
		header.data[TEXT("x-api-key")] = m_api_img_key;
		
		// form
		av::http::File file;
		file.data[TEXT("source")] = img_path;

		// send
		if (!client.postForm(m_api_img_url, std::make_tuple(header, file), resp)) {
			loge("post form failed");
			return false;
		}

		if (!resp.isOk()) {
			loge("post form failed, code = {}, body = {}", resp.code, av::str::toA(resp.body) );
			return false;
		}

		logi("{}", av::str::toA(resp.body) );

		json j;
		try {
			auto r = j.parse(av::str::toA(resp.body));
			if (!r.contains("status_code")) {
				loge("no status_code field, {}", av::str::toA(resp.body));
				return false;
			}
			if (!r.contains("image")) {
				loge("no image field, {}", av::str::toA(resp.body));
				return false;
			}

			if (!r["image"].contains("url") || !r["image"]["url"].is_string()) {
				loge("image  no url field, {}", av::str::toA(resp.body));
				return false;
			}
			auto status_code = r["status_code"].get<int64_t>();
			auto image_url = r["image"]["url"].get<std::string>();

			// check
			if (status_code != 200) {
				loge("status_code failed, {}", av::str::toA(resp.body));
				return false;
			}

			url = av::str::toT(image_url);
			return true;
		}
		catch (const nlohmann::json::parse_error& e) {
			loge("parse_error {}, {}", e.what(), av::str::toA(resp.body));
			return false;
		}
		catch (const std::exception& e) {
			loge("parse_error {}, {}", e.what(), av::str::toA(resp.body));
			return false;
		}
		return false;
	}
}

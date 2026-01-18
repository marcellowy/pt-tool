#include <tuple>
#include "upload_img.h"
#include "av_http.h"
#include "av_log.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace http_ = av::http;

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
		http_::Client client;

		// header
		http_::Header header;
		header.kv["x-api-key"] = av::str::toA(m_api_img_key);
		
		// form
		http_::Form form;
		form.file["source"] = av::str::toA(img_path);

		// send
		const auto resp = client.post(av::str::toA(m_api_img_url), header, form);
		if (!resp) {
			loge("post form failed");
			return false;
		}

		if (resp->status != 200) {
			loge("post form failed, code = {}, body = {}", resp->status, resp->body );
			return false;
		}

		logi("{}", resp->body);

		json j;
		try {
			auto r = j.parse(resp->body);
			if (!r.contains("status_code")) {
				loge("no status_code field, {}", resp->body);
				return false;
			}
			if (!r.contains("image")) {
				loge("no image field, {}", resp->body);
				return false;
			}

			if (!r["image"].contains("url") || !r["image"]["url"].is_string()) {
				loge("image  no url field, {}", resp->body);
				return false;
			}
			auto status_code = r["status_code"].get<int64_t>();
			auto image_url = r["image"]["url"].get<std::string>();

			// check
			if (status_code != 200) {
				loge("status_code failed, {}", resp->body);
				return false;
			}

			url = av::str::toT(image_url);
			return true;
		}
		catch (const nlohmann::json::parse_error& e) {
			loge("parse_error {}, {}", e.what(), resp->body);
			return false;
		}
		catch (const std::exception& e) {
			loge("parse_error {}, {}", e.what(), resp->body);
			return false;
		}
		return false;
	}
}

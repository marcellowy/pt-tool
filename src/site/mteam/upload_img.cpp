#include <tuple>
#include "upload_img.h"
#include "av_http.h"
#include "av_log.h"

namespace mteam {
	UploadImg::UploadImg(const std::tstring& key) : m_api_img_key(key)
	{
	}

	UploadImg::~UploadImg()
	{
	}


	bool UploadImg::Upload(const std::tstring& img_path) {

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
		if (!client.postForm(TEXT("https://img.m-team.cc/api/1/upload"), std::make_tuple(header, file), resp)) {
			loge("post form failed");
			return false;
		}

		if (!resp.isOk()) {
			loge("post form failed, code = {}, body = {}", resp.code, av::str::toA(resp.body) );
			return false;
		}

		logi("{}", av::str::toA(resp.body) );
		
		return true;
	}
}

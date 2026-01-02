#ifndef MTEAM_UPLOAD_IMG_H_
#define MTEAM_UPLOAD_IMG_H_

#include "av_string.h"

namespace mteam{
	class UploadImg
	{
	public:
		UploadImg(const std::tstring& url, const std::tstring& key);
		~UploadImg();
		bool Upload(const std::tstring& img_path, std::tstring& url);
	private:
		std::tstring m_api_img_url; // img api url
		std::tstring m_api_img_key; // img api key
	};
}
#endif

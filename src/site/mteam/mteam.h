#ifndef MTEAM_MTEAM_H_
#define MTEAM_MTEAM_H_

#include <memory>

#include "av_media_info.h"
#include "site.h"

#include "video_codec.h"
#include "video_resolution.h"
#include "audio_codec.h"
#include "category.h"
#include "source.h"
#include "upload_img.h"
#include "group.h"

namespace mteam {

	class Mteam : public Site
	{
	public:
		Mteam(const std::tstring& api_url,
			const std::tstring& api_key, 
			const std::tstring& img_api_url, 
			const std::tstring& img_api_key,
			const std::tstring& tgbot_token,
			const std::tstring& tgbot_chat_id);

		bool publish(const av::media::Source& source) override;
		~Mteam();
	private:
		std::tstring m_api_url;
		std::tstring m_api_key;
		std::tstring m_img_api_url;
		std::tstring m_img_api_key;
		std::tstring m_tgbot_token;
		std::tstring m_tgbot_chat_id;

		av::media::Source m_external_source;

		mteam::video::Codec m_video_codec;
		mteam::video::Resolution m_video_resolution;
		mteam::audio::Codec m_audio_codec;
		mteam::Category m_cateogry;
		int64_t group_id = 0;
		mteam::Source m_source;
	private:
		bool sendTGMessage(std::tstring& douban_poster_img,
			std::vector<std::tstring>& screenshots,
			std::tstring& publish_id,
			std::tstring& title,
			std::tstring& sub_title,
			std::tstring& create_date);
		bool parseName(std::tstring& torrent_dir,
			std::tstring& title,
			std::tstring& video_filename,
			std::tstring& torrent_filename
		);
	};

}
#endif

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
	class Mteam : public Site {
	public:
		Mteam(const std::tstring &api_url,
		      const std::tstring &api_key,
		      const std::tstring &img_api_url,
		      const std::tstring &img_api_key,
		      const std::tstring &tgbot_token,
		      const std::tstring &tgbot_chat_id,
		      const bool& debug);

		bool publish(const av::media::Source &source) override;
		bool name(const av::media::Source& source, PublishName& publish_name) override;

		~Mteam() override;

	private:
		std::tstring m_api_url;
		std::tstring m_api_key;
		std::tstring m_img_api_url;
		std::tstring m_img_api_key;
		std::tstring m_tgbot_token;
		std::tstring m_tgbot_chat_id;

		mteam::video::Codec m_video_codec;
		mteam::video::Resolution m_video_resolution;
		mteam::audio::Codec m_audio_codec;
		mteam::Category m_category;
		int64_t group_id = 0;
		mteam::Source m_source;

		//
		bool m_debug = false;

	private:
		bool sendTGMessage(const std::tstring &douban_poster_img,
		                   const std::vector<std::tstring> &screenshots,
		                   const std::tstring &publish_id,
		                   const std::tstring &title,
		                   const std::tstring &sub_title,
		                   const std::tstring &create_date);

		void sendTGWaringMessage(const std::tstring &msg) const;
	};
}
#endif

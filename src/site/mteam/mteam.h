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
		Mteam();
		bool publish(const av::media::Source& source) override;
		~Mteam();
	private:
		av::media::Source m_external_source;

		mteam::video::Codec m_video_codec;
		mteam::video::Resolution m_video_resolution;
		mteam::audio::Codec m_audio_codec;
		mteam::Category m_cateogry;
		int64_t group_id;
		mteam::Source m_source;
	private:
		bool parseName();
	};

}
#endif

#ifndef MTEAM_VIDEO_CODEC_H_
#define MTEAM_VIDEO_CODEC_H_

#include <map>

#include "av_string.h"

#include "av_media_info.h"


namespace mteam {
	enum class VideoCodecId {
		Unknown = 0,
		_h264 = 1,
		_h265 = 16,
		_vc1 = 2,
		_mpeg2 = 4,
		_xvid = 3,
		_av1 = 19,
		_vp8 = 21,
		_vp9 = 21,
		_avs = 22,
		_avs2 = 22,
		_cavs = 22,
	};

	class VideoCodec
	{
	public:
		VideoCodec() = default;
		VideoCodec(const av::media::SourceVideoCodec& codec);
		~VideoCodec();
		VideoCodecId getid();
		std::tstring getText();
	private:
		av::media::SourceVideoCodec m_codec;
	};

}

#endif
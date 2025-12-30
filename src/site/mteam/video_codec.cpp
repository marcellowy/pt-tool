#include "video_codec.h"

namespace mteam {

	static std::map<VideoCodecId, std::tstring> VideoCodecIdMapString = {
		{VideoCodecId::Unknown, TEXT("Other")},
		{VideoCodecId::_h264, TEXT("H.264")},
		{VideoCodecId::_h265, TEXT("H.265")},
		{VideoCodecId::_vc1, TEXT("VC-1")},
		{VideoCodecId::_mpeg2, TEXT("MPEG-2")},
		{VideoCodecId::_xvid, TEXT("Xvid")},
		{VideoCodecId::_av1, TEXT("AV1")},
		{VideoCodecId::_vp8, TEXT("VP8/9")},
		{VideoCodecId::_vp9, TEXT("VP8/9")},
		{VideoCodecId::_avs, TEXT("AVS")},
		{VideoCodecId::_avs2, TEXT("AVS")},
		{VideoCodecId::_cavs, TEXT("AVS")},
	};

	VideoCodec::VideoCodec(const av::media::SourceVideoCodec& codec): m_codec(codec)
	{
	}

	VideoCodec::~VideoCodec()
	{
	}

	VideoCodecId VideoCodec::getid() {
		switch (m_codec) {
		case av::media::SourceVideoCodec::_h264:
			return VideoCodecId::_h264;
			break;
		case av::media::SourceVideoCodec::_h265:
			return VideoCodecId::_h265;
			break;
		case av::media::SourceVideoCodec::_vc1:
			return VideoCodecId::_vc1;
			break;
		case av::media::SourceVideoCodec::_mpeg2:
			return VideoCodecId::_mpeg2;
			break;
		case av::media::SourceVideoCodec::_xvid:
			return VideoCodecId::_xvid;
			break;
		case av::media::SourceVideoCodec::_av1:
			return VideoCodecId::_av1;
			break;
		case av::media::SourceVideoCodec::_vp8:
			return VideoCodecId::_vp8;
			break;
		case av::media::SourceVideoCodec::_vp9:
			return VideoCodecId::_vp9;
			break;
		case av::media::SourceVideoCodec::_avs:
			return VideoCodecId::_avs;
			break;
		case av::media::SourceVideoCodec::_avs2:
			return VideoCodecId::_avs2;
			break;
		case av::media::SourceVideoCodec::_cavs:
			return VideoCodecId::_cavs;
			break;
		}
		return VideoCodecId::Unknown;
	}

	std::tstring VideoCodec::getText() {
		auto id = getid();
		if (VideoCodecIdMapString.find(id) != VideoCodecIdMapString.end()) {
			return VideoCodecIdMapString[id];
		}
		return VideoCodecIdMapString[VideoCodecId::Unknown];
	}

}
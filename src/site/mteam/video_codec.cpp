#include "video_codec.h"

namespace mteam {
	namespace video {
		static std::map<CodecId, std::tstring> CodecIdMapString = {
			{CodecId::Unknown, TEXT("Other")},
			{CodecId::_h264, TEXT("H.264")},
			{CodecId::_h265, TEXT("H.265")},
			{CodecId::_vc1, TEXT("VC-1")},
			{CodecId::_mpeg2, TEXT("MPEG-2")},
			{CodecId::_xvid, TEXT("Xvid")},
			{CodecId::_av1, TEXT("AV1")},
			{CodecId::_vp8, TEXT("VP8/9")},
			{CodecId::_vp9, TEXT("VP8/9")},
			{CodecId::_avs, TEXT("AVS")},
			{CodecId::_avs2, TEXT("AVS")},
			{CodecId::_cavs, TEXT("AVS")},
		};

		Codec::Codec(const av::media::SourceVideoCodec& codec) : m_codec(codec)
		{
		}

		Codec::~Codec()
		{
		}

		CodecId Codec::getid() {
			switch (m_codec) {
			case av::media::SourceVideoCodec::_h264:
				return CodecId::_h264;
				break;
			case av::media::SourceVideoCodec::_h265:
				return CodecId::_h265;
				break;
			case av::media::SourceVideoCodec::_vc1:
				return CodecId::_vc1;
				break;
			case av::media::SourceVideoCodec::_mpeg2:
				return CodecId::_mpeg2;
				break;
			case av::media::SourceVideoCodec::_xvid:
				return CodecId::_xvid;
				break;
			case av::media::SourceVideoCodec::_av1:
				return CodecId::_av1;
				break;
			case av::media::SourceVideoCodec::_vp8:
				return CodecId::_vp8;
				break;
			case av::media::SourceVideoCodec::_vp9:
				return CodecId::_vp9;
				break;
			case av::media::SourceVideoCodec::_avs:
				return CodecId::_avs;
				break;
			case av::media::SourceVideoCodec::_avs2:
				return CodecId::_avs2;
				break;
			case av::media::SourceVideoCodec::_cavs:
				return CodecId::_cavs;
				break;
			}
			return CodecId::Unknown;
		}

		std::tstring Codec::getText() {
			auto id = getid();
			if (CodecIdMapString.find(id) != CodecIdMapString.end()) {
				return CodecIdMapString[id];
			}
			return CodecIdMapString[CodecId::Unknown];
		}
	}
}

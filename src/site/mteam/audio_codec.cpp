#include "audio_codec.h"

namespace mteam {
	namespace audio {
		
		// SourceAudioCodec to string
		static std::map<av::media::SourceAudioCodec, std::tstring> SourceAudioCodecMapString = {
			{av::media::SourceAudioCodec::Unknown, TEXT("Other")},
			{av::media::SourceAudioCodec::_aac, TEXT("AAC")},
			{av::media::SourceAudioCodec::_ac3, TEXT("AC3")},// AC3(DD)
			{av::media::SourceAudioCodec::_dts, TEXT("DTS")},
			{av::media::SourceAudioCodec::_dts_hd_ma, TEXT("DTS-HD MA")},
			{av::media::SourceAudioCodec::_e_ac3_ddp, TEXT("E-AC3")}, // E-AC3(DDP)
			{av::media::SourceAudioCodec::_e_ac3_atmos, TEXT("E-AC3 Atmos")}, // E-AC3 Atmos(DDP Atmos)
			{av::media::SourceAudioCodec::_true_hd, TEXT("TrueHD)")},
			{av::media::SourceAudioCodec::_true_hd_atmos, TEXT("TrueHD Atmos")},
			{av::media::SourceAudioCodec::_lpcm, TEXT("LPCM/PCM")}, // LPCM/PCM
			{av::media::SourceAudioCodec::_wav, TEXT("WAV")},
			{av::media::SourceAudioCodec::_flac, TEXT("FLAC")},
			{av::media::SourceAudioCodec::_ape, TEXT("APE")},
			{av::media::SourceAudioCodec::_mp1, TEXT("MP1")},
			{av::media::SourceAudioCodec::_mp2, TEXT("MP2")},
			{av::media::SourceAudioCodec::_mp3, TEXT("MP3")},
			{av::media::SourceAudioCodec::_ogg, TEXT("OGG")},
			{av::media::SourceAudioCodec::_other, TEXT("Other")},
		};

		Codec::Codec(const av::media::SourceAudioCodec& codec) :m_codec(codec) {}
		Codec::~Codec() {}

		void Codec::setSourceCodec(const av::media::SourceAudioCodec& codec) {
			m_codec = codec;
		}

		CodecId Codec::getid() {
			switch (m_codec) {
			case av::media::SourceAudioCodec::_aac:
				return CodecId::_aac;
				break;
			case av::media::SourceAudioCodec::_ac3:
				return CodecId::_ac3;
				break;
			case av::media::SourceAudioCodec::_dts:
				return CodecId::_dts;
				break;
			case av::media::SourceAudioCodec::_dts_hd_ma:
				return CodecId::_dts_hd_ma;
				break;
			case av::media::SourceAudioCodec::_e_ac3_ddp:
				return CodecId::_e_ac3_ddp;
				break;
				case av::media::SourceAudioCodec::_e_ac3_atmos:
				return CodecId::_e_ac3_atmos;
				break;
			case av::media::SourceAudioCodec::_true_hd:
				return CodecId::_true_hd;
				break;
			case av::media::SourceAudioCodec::_true_hd_atmos:
				return CodecId::_true_hd_atmos;
				break;
			case av::media::SourceAudioCodec::_lpcm:
				return CodecId::_lpcm;
				break;
			case av::media::SourceAudioCodec::_wav:
				return CodecId::_wav;
				break;
			case av::media::SourceAudioCodec::_flac:
				return CodecId::_flac;
				break;
			case av::media::SourceAudioCodec::_ape:
				return CodecId::_ape;
				break;
			case av::media::SourceAudioCodec::_mp1:
				return CodecId::_mp1;
				break;
			case av::media::SourceAudioCodec::_mp2:
				return CodecId::_mp2;
				break;
			case av::media::SourceAudioCodec::_mp3:
				return CodecId::_mp3;
				break;
			case av::media::SourceAudioCodec::_ogg:
				return CodecId::_ogg;
				break;
			case av::media::SourceAudioCodec::_other:
				return CodecId::_other;
				break;
			case av::media::SourceAudioCodec::Unknown:
				return CodecId::Unknown;
				break;
			}
			return CodecId::Unknown;
		}

		std::tstring Codec::getText() {
			if(SourceAudioCodecMapString.contains(m_codec)) {
				return SourceAudioCodecMapString[m_codec];
			}
			return SourceAudioCodecMapString[av::media::SourceAudioCodec::Unknown];
		}
	}
}


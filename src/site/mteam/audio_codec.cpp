#include "audio_codec.h"

namespace mteam {
	namespace audio {
		static std::map<CodecId, std::tstring> CodecIdMapString = {
			{CodecId::Unknown, TEXT("Other")},
			{CodecId::_aac, TEXT("AAC")},
			{CodecId::_ac3, TEXT("AC3(DD)")},
			{CodecId::_dts, TEXT("DTS")},
			{CodecId::_dts_hd_ma, TEXT("DTS-HD MA")},
			{CodecId::_e_ac3_ddp, TEXT("E-AC3(DDP)")},
			{CodecId::_e_ac3_atoms, TEXT("E-AC3 Atoms(DDP Atoms)")},
			{CodecId::_true_hd, TEXT("TrueHD)")},
			{CodecId::_true_hd_atmos, TEXT("TrueHD Atmos")},
			{CodecId::_lpcm, TEXT("LPCM/PCM")},
			{CodecId::_wav, TEXT("WAV")},
			{CodecId::_flac, TEXT("FLAC")},
			{CodecId::_ape, TEXT("APE")},
			{CodecId::_mp1, TEXT("MP1")},
			{CodecId::_mp2, TEXT("MP2")},
			{CodecId::_mp3, TEXT("MP3")},
			{CodecId::_ogg, TEXT("OGG")},
			{CodecId::_other, TEXT("Other")},
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
			case av::media::SourceAudioCodec::_e_ac3_atoms:
				return CodecId::_e_ac3_atoms;
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


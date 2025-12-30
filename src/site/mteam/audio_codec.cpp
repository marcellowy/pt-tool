#include "audio_codec.h"

namespace mteam {

	static std::map<AudioCodecId, std::tstring> AudioCodecIdMapString = {
		{AudioCodecId::Unknown, TEXT("Other")},
		{AudioCodecId::_aac, TEXT("AAC")},
		{AudioCodecId::_ac3, TEXT("AC3(DD)")},
		{AudioCodecId::_dts, TEXT("DTS")},
		{AudioCodecId::_dts_hd_ma, TEXT("DTS-HD MA")},
		{AudioCodecId::_e_ac3_ddp, TEXT("E-AC3(DDP)")},
		{AudioCodecId::_e_ac3_atoms, TEXT("E-AC3 Atoms(DDP Atoms)")},
		{AudioCodecId::_true_hd, TEXT("TrueHD)")},
		{AudioCodecId::_true_hd_atmos, TEXT("TrueHD Atmos")},
		{AudioCodecId::_lpcm, TEXT("LPCM/PCM")},
		{AudioCodecId::_wav, TEXT("WAV")},
		{AudioCodecId::_flac, TEXT("FLAC")},
		{AudioCodecId::_ape, TEXT("APE")},
		{AudioCodecId::_mp1, TEXT("MP1")},
		{AudioCodecId::_mp2, TEXT("MP2")},
		{AudioCodecId::_mp3, TEXT("MP3")},
		{AudioCodecId::_ogg, TEXT("OGG")},
		{AudioCodecId::_other, TEXT("Other")},
	};

	AudioCodec::AudioCodec(const av::media::SourceAudioCodec& codec):m_codec(codec) {}
	AudioCodec::~AudioCodec() {}

	AudioCodecId AudioCodec::getid() {
		switch (m_codec) {
		case av::media::SourceAudioCodec::_aac:
			return AudioCodecId::_aac;
			break;
		case av::media::SourceAudioCodec::_ac3:
			return AudioCodecId::_ac3;
			break;
		case av::media::SourceAudioCodec::_dts:
			return AudioCodecId::_dts;
			break;
		case av::media::SourceAudioCodec::_dts_hd_ma:
			return AudioCodecId::_dts_hd_ma;
			break;
		case av::media::SourceAudioCodec::_e_ac3_ddp:
			return AudioCodecId::_e_ac3_ddp;
			break;
		case av::media::SourceAudioCodec::_e_ac3_atoms:
			return AudioCodecId::_e_ac3_atoms;
			break;
		case av::media::SourceAudioCodec::_true_hd:
			return AudioCodecId::_true_hd;
			break;
		case av::media::SourceAudioCodec::_true_hd_atmos:
			return AudioCodecId::_true_hd_atmos;
			break;
		case av::media::SourceAudioCodec::_lpcm:
			return AudioCodecId::_lpcm;
			break;
		case av::media::SourceAudioCodec::_wav:
			return AudioCodecId::_wav;
			break;
		case av::media::SourceAudioCodec::_flac:
			return AudioCodecId::_flac;
			break;
		case av::media::SourceAudioCodec::_ape:
			return AudioCodecId::_ape;
			break;
		case av::media::SourceAudioCodec::_mp1:
			return AudioCodecId::_mp1;
			break;
		case av::media::SourceAudioCodec::_mp2:
			return AudioCodecId::_mp2;
			break;
		case av::media::SourceAudioCodec::_mp3:
			return AudioCodecId::_mp3;
			break;
		case av::media::SourceAudioCodec::_ogg:
			return AudioCodecId::_ogg;
			break;
		case av::media::SourceAudioCodec::_other:
			return AudioCodecId::_other;
			break;
		}
		return AudioCodecId::Unknown;
	}

	std::tstring AudioCodec::getText() {
		auto id = getid();
		if (AudioCodecIdMapString.find(id) != AudioCodecIdMapString.end()) {
			return AudioCodecIdMapString[id];
		}
		return AudioCodecIdMapString[AudioCodecId::Unknown];
	}
}


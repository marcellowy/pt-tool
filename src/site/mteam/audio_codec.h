#ifndef MTEAM_AUDIO_CODEC_H_
#define MTEAM_AUDIO_CODEC_H_

#include <map>

#include "av_string.h"

#include "av_media_info.h"


namespace mteam {
	namespace audio {
		enum class CodecId {
			Unknown = 0,
			_aac = 6,
			_ac3 = 8,
			_dts = 3,
			_dts_hd_ma = 11,
			_e_ac3_ddp = 12,
			_e_ac3_atoms = 13,
			_true_hd = 9,
			_true_hd_atmos = 10,
			_lpcm = 14,
			_wav = 15,
			_flac = 1,
			_ape = 2,
			_mp1 = 4,
			_mp2 = 4,
			_mp3 = 4,
			_ogg = 5,
			_other = 7,
		};

		class Codec
		{
		public:
			Codec() = default;
			Codec(const av::media::SourceAudioCodec& codec);
			~Codec();
			void setSourceCodec(const av::media::SourceAudioCodec& codec);
			CodecId getid();
			std::tstring getText();
		private:
			av::media::SourceAudioCodec m_codec;
		};
	}
}

#endif

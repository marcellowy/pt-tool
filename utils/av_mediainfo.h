#ifndef AV_MEDIAINFO_H_
#define AV_MEDIAINFO_H_

#include "nlohmann/json.hpp"
#include "av_string.h"
#include "av_media_info.h"

namespace av {
	namespace mediainfo {

		struct General {
		};

		struct Video {
			av::media::SourceVideoCodec codec = av::media::SourceVideoCodec::Unknown;
			int64_t width;
			int64_t height;
			av::media::ScanType scan_type = av::media::ScanType::Unknown;
		};

		struct Audio {
			av::media::SourceAudioCodec codec = av::media::SourceAudioCodec::Unknown;
		};

		class MediaInfo {
		public:
			MediaInfo(const std::tstring& path);
			~MediaInfo();
			bool parse();
			Video getVideo();
			Audio getAudio();
		protected:
			std::tstring m_video_path;
			std::tstring m_json;
			std::tstring m_text;
			Video m_video;
			Audio m_audio;
		};
	}
}
#endif
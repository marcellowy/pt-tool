#ifndef AV_MEDIAINFO_H_
#define AV_MEDIAINFO_H_

#include "nlohmann/json.hpp"
#include "av_string.h"

namespace av {
	namespace mediainfo {

		struct General {
		};

		struct Video {
			std::tstring format;
			int64_t width;
			int64_t height;
			std::tstring scan_type;
		};

		struct Audio {
			std::tstring format;
			std::tstring format_commercial_if_any;
			std::tstring format_profile;
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
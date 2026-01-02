#ifndef MTEAM_VIDEO_RESOLUTION_H_
#define MTEAM_VIDEO_RESOLUTION_H_

#include <map>

#include "av_string.h"

#include "av_media_info.h"


namespace mteam {
	namespace video {

		enum class ResolutionId {
			Unknown = 0,
			_480 = 0, // mteam 没有这个解析度
			_720 = 4,
			_1080p = 1,
			_1080i = 2,
			_2k = 0, // mteam 没有这个解析度
			_4k = 6,
			_8k = 7
		};

		class Resolution
		{
		public:
			Resolution() = default;
			Resolution(const av::media::SourceVideoResolution& resolution);
			~Resolution();
			ResolutionId getid();
			std::tstring getText();
		private:
			av::media::SourceVideoResolution m_resolution;
		};
	}
}

#endif

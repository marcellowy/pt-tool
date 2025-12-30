#ifndef AV_FFMPEG_H_
#define AV_FFMPEG_H_

#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <cstdint>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
}

#include "av_string.h"
#include "av_codec.h"

namespace av {
	namespace ffmpeg {
		bool captureFrame(const std::tstring& video, const std::vector<int64_t>& time_seconds, av::codec::Codec& codec);
	}
}

#endif

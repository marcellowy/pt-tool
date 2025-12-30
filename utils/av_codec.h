#ifndef AV_CODEC_H_
#define AV_CODEC_H_

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

namespace av {
	namespace codec {
		class Codec
		{
		public:
			virtual bool init(AVCodecContext* codec_ctx) = 0;
			virtual bool codec(AVFrame* frame) = 0;
		};
	}
}

#endif
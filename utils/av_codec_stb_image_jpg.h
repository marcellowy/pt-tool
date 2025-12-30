#ifndef AV_CODEC_STB_PNG_H_
#define AV_CODEC_STB_PNG_H_

#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <cstdint>

extern "C" {
#include "libavformat/avformat.h"
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
	namespace codec {
		class StbPNG : public Codec {
		public:
			StbPNG(std::function<void(void* data, int size)> callback);
			~StbPNG();
			bool init(AVCodecContext* codec_ctx);
			bool codec(AVFrame* frame);
		private:
			void destory();
		private:
			AVCodecContext* m_codec_ctx = NULL;
			const AVPixelFormat m_format = AV_PIX_FMT_RGB24;
			struct SwsContext* m_sws_ctx = NULL;
			std::function<void(void* data, int size)> m_callback = nullptr;
		};
	}
}

#endif
#include "av_codec_stb_image_jpg.h"

#include "av_log.h"

// 在test.cpp中定义STB_IMAGE_IMPLEMENTATION
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

// 在main.cpp中定义STB_IMAGE_WRITE_IMPLEMENTATION
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

namespace av {
    namespace codec {

        void write_callback_wrapper(void* context, void* data, int size) {
            auto* callback = static_cast<std::function<void(void*, void*, int)>*>(context);
            (*callback)(context, data, size);
        }

        StbPNG::StbPNG(std::function<void(void* data, int size)> callback) :m_callback(std::move(callback)) {

        }

        StbPNG::~StbPNG() {
            destory();
        }

        void StbPNG::destory() {
            if(m_sws_ctx != NULL) sws_freeContext(m_sws_ctx);
        }

        bool StbPNG::init(AVCodecContext* codec_ctx) {
            m_codec_ctx = codec_ctx;
            return true;
        }

        bool StbPNG::codec(AVFrame* frame) {

            // dst
            int dst_width = m_codec_ctx->width;
            int dst_height = m_codec_ctx->height;
            const AVPixelFormat dst_format = m_format;

            // src
            int src_width = m_codec_ctx->width;
            int src_height = m_codec_ctx->height;
            

            // sws_ctx
            if (m_sws_ctx == NULL) {
                AVPixelFormat src_fmt =
                    static_cast<AVPixelFormat>(frame->format);
                m_sws_ctx = sws_getContext(src_width, src_height, src_fmt,
                    dst_width, dst_height, dst_format,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);
            }

            // 分配内存以存储转换后的 RGB 数据
            int num_bytes = av_image_get_buffer_size(dst_format, frame->width, frame->height, 1);
            uint8_t* rgb_buffer = (uint8_t*)malloc(num_bytes);
            if (!rgb_buffer) {
                loge("error allocating memory for RGB buffer");
                sws_freeContext(m_sws_ctx);
                return false;
            }

            // 使用 sws_scale 将 YUV 数据转换为 RGB 数据
            uint8_t* src_data[4] = { frame->data[0], frame->data[1], frame->data[2], NULL };
            int src_linesize[4] = { frame->linesize[0], frame->linesize[1], frame->linesize[2], 0 };

            uint8_t* dst_data[4] = { rgb_buffer, NULL, NULL, NULL };
            int dst_linesize[4] = { frame->width * 3, 0, 0, 0 }; // 每行三个字节 (RGB)

            sws_scale(m_sws_ctx, frame->data, frame->linesize, 0, m_codec_ctx->height,
                dst_data, dst_linesize);

            std::function<void(void*, void*, int)> write_func = [this](void* context, void* data, int size) {
                m_callback(data, size);
            };

            stbi_write_png_to_func(write_callback_wrapper, &write_func, frame->width, frame->height, 3, rgb_buffer, frame->width * 3);
            free(rgb_buffer);
            return true;
        }
    }
}

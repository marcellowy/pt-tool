#include "av_codec_jpg.h"

#include "av_log.h"

namespace av {
    namespace codec {
        JPG::JPG(std::function<void(uint8_t* data, size_t size)> callback) : m_callback(std::move(callback)) {

        }

        JPG::~JPG() {
            destory();
        }

        void JPG::destory() {
            if (m_ctx != NULL) {
                avcodec_free_context(&m_ctx);
            }
            if (m_frame != NULL) {
                av_frame_free(&m_frame);
            }
            if (m_sws_ctx != NULL) {
                sws_freeContext(m_sws_ctx);
            }

            if (m_pkt != NULL) {
                av_packet_unref(m_pkt);
                av_packet_free(&m_pkt);
            }
        }

        bool JPG::init(AVCodecContext* codec_ctx) {
            m_codec_ctx = codec_ctx;

            int dst_width = m_codec_ctx->width;
            int dst_height = m_codec_ctx->height;

            av_log_set_level(AV_LOG_DEBUG);

            m_codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
            if (m_codec == NULL) {
                logw("avcodec_find_encoder(AV_CODEC_ID_MJPEG) failed!");
                return false;
            }
            m_ctx = avcodec_alloc_context3(m_codec);
            m_ctx->bit_rate = 4000000;
            m_ctx->width = dst_width;
            m_ctx->height = dst_height;
            m_ctx->time_base = { 1, 25 };
            m_ctx->framerate = { 25, 1 };
            m_ctx->pix_fmt = m_format;
            m_ctx->color_range = AVCOL_RANGE_JPEG;

            // quality
            AVDictionary* opts = nullptr;
            av_dict_set(&opts, "jpeg_quality", "100", 0); // 0 ~ 100，默认 75

            // open
            if (avcodec_open2(m_ctx, m_codec, &opts) != 0) {
                logw("can not open codec");
                return false;
            }

            // frame
            m_frame = av_frame_alloc();
            m_frame->format = m_ctx->pix_fmt;
            m_frame->width = dst_width;
            m_frame->height = dst_height;
            av_image_alloc(m_frame->data, m_frame->linesize, dst_width, dst_height, m_ctx->pix_fmt, 1);

            // pkt
            m_pkt = av_packet_alloc();
            m_pkt->data = nullptr;
            m_pkt->size = 0;
            return true;
        }

        bool JPG::codec(AVFrame* frame) {
            logi("codec ");
            int dst_width = m_codec_ctx->width;
            int dst_height = m_codec_ctx->height;

            if (m_sws_ctx == NULL) {
                AVPixelFormat src_fmt =
                    static_cast<AVPixelFormat>(frame->format);
                m_sws_ctx = sws_getContext(m_codec_ctx->width, m_codec_ctx->height, src_fmt,
                    dst_width, dst_height, m_ctx->pix_fmt,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);
            }

            sws_scale(m_sws_ctx, frame->data, frame->linesize, 0, m_codec_ctx->height,
                m_frame->data, m_frame->linesize);

            // 
            if (auto ret = avcodec_send_frame(m_ctx, m_frame); ret != 0) {
                logw("avcodec_send_frame failed! ret = {}", ret);
                return false;
            }
            if (auto ret = avcodec_receive_packet(m_ctx, m_pkt); ret != 0) {
                logw("avcodec_receive_packet failed! ret = {}", ret);
                return false;
            }

            // callback
            m_callback(m_pkt->data, m_pkt->size);
            av_packet_unref(m_pkt);

            return true;
        }
    }
}
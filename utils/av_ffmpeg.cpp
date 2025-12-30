#include "av_ffmpeg.h"

#include "av_log.h"
#include "av_string.h"
#include "av_async.h"
#include "av_path.h"
#include "av_codec_png.h"
#include "av_codec_jpg.h"
#include "av_codec.h"

#include <codecvt>

namespace av {
    namespace ffmpeg {

        bool captureFrame(const std::tstring& video, const std::vector<int64_t>& time_seconds, av::codec::Codec& external_codec)
        {
            if (!av::path::exists(video))
            {
                loge("file {} not exists", av::str::toA(video));
                return false;
            }
            AVFormatContext* fmt_ctx = nullptr;
            if (avformat_open_input(&fmt_ctx, av::str::toA(video).c_str(), nullptr, nullptr) != 0) {
                loge("avformat_open_input failed!");
                return false;
            }
            av::async::Exit exit_fmt_ctx([&fmt_ctx] {
                avformat_close_input(&fmt_ctx);
            });
            if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
                loge("avformat_find_stream_info failed");
                return false;
            }

            //
            int video_stream_index = -1;
            for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
                if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    video_stream_index = i;
                    break;
                }
            }
            if (video_stream_index == -1) {
                logw("no vidoe stream data");
                return false;
            }

            //
            AVCodecParameters* codec_par = fmt_ctx->streams[video_stream_index]->codecpar;
            const AVCodec* codec = avcodec_find_decoder(codec_par->codec_id);
            if (!codec) {
                logw("no video decoder");
                return false;
            }

            AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(codec_ctx, codec_par);
            if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
                logw("can not open decoder");
                return false;
            }
            av::async::Exit exit_codec_ctx([&codec_ctx] {
                avcodec_free_context(&codec_ctx);
                });

            AVPacket* pkt = av_packet_alloc();
            AVFrame* frame = av_frame_alloc();
            av::async::Exit exit_frame([&frame, &pkt] {
                if (frame != nullptr) av_frame_free(&frame);
                if (pkt != nullptr) av_packet_free(&pkt);
                });

            if (!external_codec.init(codec_ctx)) {
                loge("external_codec failed");
                return false;
            }

            // read video duration
            AVStream* st;
            int i;
            int64_t duration = 0;
            for (i = 0; i < fmt_ctx->nb_streams; i++) {
                st = fmt_ctx->streams[i];
                if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
                    duration = st->duration * AV_TIME_BASE;
                    break;
                }   
            }

            // 
            for (int i = 0; i < time_seconds.size(); i++) {
                auto timebase = time_seconds[i] * AV_TIME_BASE;
                if (timebase > duration) {
                    logw("timebase {} > duration {}", timebase, duration);
                    continue;
                }

                avcodec_flush_buffers(codec_ctx);
                av_seek_frame(fmt_ctx, -1, timebase, AVSEEK_FLAG_BACKWARD); // AVSEEK_FLAG_BACKWARD to key frame

                //
                while (av_read_frame(fmt_ctx, pkt) >= 0) {
                    if (pkt->stream_index == video_stream_index) {
                        if (avcodec_send_packet(codec_ctx, pkt) == 0) {
                            if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                                if (!external_codec.codec(frame)) {
                                    loge("external_codec.codec failed!");
                                }
                                break;
                            }
                        }
                    }
                    av_packet_unref(pkt);
                }
            }
            return true;
        }
    }
}
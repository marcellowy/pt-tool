/*
 * file matroska_ffmpeg_save_frames.cpp
 * @brief 解封装（FFmpeg，参考 mpv 实现）+ FFmpeg 解码 + 保存帧为 PNG 图片（stb_image_write）
 *
 * 对外接口见 matroska_ffmpeg_save_frames.h，可直接被其他模块调用。
 * 依赖：FFmpeg (avformat, avcodec, avutil, swscale), stb_image_write
 *
 * 使用 FFmpeg 的 avformat API 进行解封装，而不是直接使用 libmatroska。
 * 这样可以更好地处理压缩的 Tracks 元素和其他复杂的 Matroska 特性。
 */

#include "matroska_ffmpeg_save_frames.h"
#include <cstdio>
#include "stb_image_write.h"
#include <cstdlib>
#include <cstring>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libswscale/swscale.h>
}


/* ---------- FFmpeg 解码 + 保存 PNG ---------- */
struct DecodeAndSaveContext {
    AVCodecContext *dec_ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    struct SwsContext *sws = nullptr;
    AVPixelFormat sws_input_fmt = AV_PIX_FMT_NONE; // 记录 SWS context 的输入格式
    AVFrame *rgb_frame = nullptr;
    uint8_t *rgb_buf = nullptr;
    char path_prefix[512];
    size_t path_prefix_len = 0;
    int width = 0, height = 0;
    int frame_index = 0;
    int max_frames = 0;
};


/** 使用 stb_image_write 直接保存 RGB24 为 PNG */
static bool writeFrameAsPng(DecodeAndSaveContext *ctx) {
    if (!ctx->rgb_frame || !ctx->rgb_frame->data[0]) {
        fprintf(stderr, "[writeFrameAsPng] ERROR: rgb_frame or data[0] is null!\n");
        return false;
    }
    if (ctx->width <= 0 || ctx->height <= 0) {
        fprintf(stderr, "[writeFrameAsPng] ERROR: Invalid dimensions: %dx%d\n", ctx->width, ctx->height);
        return false;
    }

    snprintf(ctx->path_prefix + ctx->path_prefix_len,
             sizeof(ctx->path_prefix) - ctx->path_prefix_len, "_%05d.png", ctx->frame_index);
    int ok = stbi_write_png(ctx->path_prefix, ctx->width, ctx->height, 3,
                            ctx->rgb_frame->data[0], ctx->rgb_frame->linesize[0]);
    ctx->path_prefix[ctx->path_prefix_len] = '\0';
    return ok != 0;
}

static void decodeAndSaveOneFrame(DecodeAndSaveContext *ctx) {
    if (ctx->frame_index >= ctx->max_frames) {
        fprintf(stderr, "[decodeAndSaveOneFrame] Frame index %d >= max_frames %d, skipping.\n", ctx->frame_index,
                ctx->max_frames);
        return;
    }
    if (!ctx->rgb_frame) {
        fprintf(stderr, "[decodeAndSaveOneFrame] ERROR: rgb_frame is null!\n");
        return;
    }
    if (!ctx->frame || !ctx->frame->data[0]) {
        fprintf(stderr, "[decodeAndSaveOneFrame] ERROR: frame or frame->data[0] is null!\n");
        return;
    }

    AVPixelFormat frame_fmt = (AVPixelFormat) ctx->frame->format;

    // 如果 SWS context 不存在或格式不匹配，重新创建
    if (!ctx->sws || ctx->sws_input_fmt != frame_fmt) {
        if (ctx->sws) {
            sws_freeContext(ctx->sws);
            ctx->sws = nullptr;
        }
        ctx->sws = sws_getContext(ctx->width, ctx->height, frame_fmt,
                                  ctx->width, ctx->height, AV_PIX_FMT_RGB24,
                                  SWS_BILINEAR | SWS_ACCURATE_RND | SWS_FULL_CHR_H_INT,
                                  nullptr, nullptr, nullptr);
        if (!ctx->sws) {
            return;
        }

        // 参考 mpv 实现：正确获取颜色空间信息
        AVColorSpace colorspace = ctx->frame->colorspace;
        int color_range = ctx->frame->color_range;
        int color_primaries = ctx->frame->color_primaries;
        int color_trc = ctx->frame->color_trc;

        // 如果未指定，使用默认值（参考 mpv）
        if (colorspace == AVCOL_SPC_UNSPECIFIED) colorspace = AVCOL_SPC_BT709;
        if (color_range == 0) color_range = 1; // 默认 limited range
        if (color_primaries == 0) color_primaries = 1; // 默认 BT709
        if (color_trc == 0) color_trc = 1; // 默认 BT709

        // srcRange: 1=full range, 0=limited range
        // 对于 limited range (16-235)，需要扩展为 full range (0-255)
        int srcRange = (color_range == 2) ? 1 : 0; // 2=JPEG/full, 1=MPEG/limited
        // dstRange: 1=full range (RGB 总是 full range)
        int dstRange = 1;

        // 选择颜色空间系数（参考 mpv）
        int sws_colorspace = SWS_CS_DEFAULT;
        if (colorspace == AVCOL_SPC_BT709) {
            sws_colorspace = SWS_CS_ITU709;
        } else if (colorspace == AVCOL_SPC_BT2020_NCL || colorspace == AVCOL_SPC_BT2020_CL) {
            sws_colorspace = SWS_CS_BT2020;
        } else if (colorspace == AVCOL_SPC_BT470BG || colorspace == AVCOL_SPC_SMPTE170M) {
            sws_colorspace = SWS_CS_ITU601;
        }

        // 输入颜色空间系数
        const int *inv_table = sws_getCoefficients(sws_colorspace);
        // 输出颜色空间系数：始终输出到 BT709 (SDR)
        const int *table = sws_getCoefficients(SWS_CS_ITU709);

        // 设置颜色空间细节（参考 mpv 和 FFmpeg 文档）
        // 参数：sws, inv_table, srcRange, table, dstRange, brightness, contrast, saturation
        // brightness, contrast, saturation 都设为 0 表示不调整
        sws_setColorspaceDetails(ctx->sws, inv_table, srcRange, table, dstRange, 0, 1 << 16, 1 << 16);
        ctx->sws_input_fmt = frame_fmt;
    }

    if (!ctx->rgb_frame->data[0]) {
        return;
    }

    int ret = sws_scale(ctx->sws, ctx->frame->data, ctx->frame->linesize, 0, ctx->height,
                        ctx->rgb_frame->data, ctx->rgb_frame->linesize);
    if (ret <= 0) {
        return;
    }

    // 强制处理 limited range 到 full range 的扩展
    // 参考 mpv：对于 limited range 视频，必须手动扩展到 full range
    int color_range = ctx->frame->color_range;
    if (color_range == 0) color_range = 1; // 默认 limited range

    // 对于 limited range 视频，强制进行范围扩展
    // limited range (16-235) 需要扩展到 full range (0-255)
    // 公式: full = (limited - 16) * 255 / (235 - 16) = (limited - 16) * 255 / 219
    if (color_range == 1) {
        // limited range
        uint8_t *rgb_data = ctx->rgb_frame->data[0];
        int stride = ctx->rgb_frame->linesize[0];

        // 对所有像素进行范围扩展
        for (int y = 0; y < ctx->height; y++) {
            uint8_t *row = rgb_data + y * stride;
            for (int x = 0; x < ctx->width; x++) {
                int r = row[x * 3];
                int g = row[x * 3 + 1];
                int b = row[x * 3 + 2];

                // 扩展公式：full = (limited - 16) * 255 / 219
                // 对于已经在 full range 的值（> 235），保持不变
                if (r <= 235) {
                    r = (int) ((r - 16) * 255.0f / 219.0f + 0.5f);
                }
                if (g <= 235) {
                    g = (int) ((g - 16) * 255.0f / 219.0f + 0.5f);
                }
                if (b <= 235) {
                    b = (int) ((b - 16) * 255.0f / 219.0f + 0.5f);
                }

                // 限制在 0-255 范围内
                row[x * 3] = (uint8_t) (r < 0 ? 0 : (r > 255 ? 255 : r));
                row[x * 3 + 1] = (uint8_t) (g < 0 ? 0 : (g > 255 ? 255 : g));
                row[x * 3 + 2] = (uint8_t) (b < 0 ? 0 : (b > 255 ? 255 : b));
            }
        }
    }

    writeFrameAsPng(ctx);
    ctx->frame_index++;
}


static void freeDecoderAndSws(DecodeAndSaveContext *ctx) {
    if (ctx->rgb_frame) av_frame_free(&ctx->rgb_frame);
    if (ctx->rgb_buf) av_freep(&ctx->rgb_buf);
    if (ctx->sws) sws_freeContext(ctx->sws);
    if (ctx->frame) av_frame_free(&ctx->frame);
    if (ctx->packet) av_packet_free(&ctx->packet);
    if (ctx->dec_ctx) avcodec_free_context(&ctx->dec_ctx);
}

/* ---------- 使用 FFmpeg demuxer（参考 mpv 的实现方式） ---------- */
extern "C" int matroska_ffmpeg_save_frames(const char *input_mkv,
                                           const char *output_prefix,
                                           int max_frames) {
    if (!input_mkv || !output_prefix) return -1;
    if (max_frames <= 0) max_frames = 30;

    // 使用 FFmpeg 的 avformat API 打开文件（与 mpv 一致）
    AVFormatContext *fmt_ctx = nullptr;
    const AVInputFormat *ifmt = av_find_input_format("matroska");
    int ret = avformat_open_input(&fmt_ctx, input_mkv, nullptr, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        fprintf(stderr, "[matroska_ffmpeg_save_frames] avformat_open_input failed: %s\n", errbuf);
        return -1;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] avformat_find_stream_info failed\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 查找视频流
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] no video stream found\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }
    int video_stream_index = ret;
    AVStream *stream = fmt_ctx->streams[video_stream_index];
    AVCodecParameters *par = stream->codecpar;

    fprintf(stderr, "[matroska_ffmpeg_save_frames] Found video stream: index=%d, codec=%d, %dx%d\n",
            video_stream_index, par->codec_id, par->width, par->height);
    fflush(stderr);

    // 初始化解码器
    DecodeAndSaveContext save_ctx = {};
    save_ctx.max_frames = max_frames;
    strncpy(save_ctx.path_prefix, output_prefix, sizeof(save_ctx.path_prefix) - 1);
    save_ctx.path_prefix[sizeof(save_ctx.path_prefix) - 1] = '\0';
    save_ctx.path_prefix_len = strlen(save_ctx.path_prefix);
    save_ctx.width = par->width;
    save_ctx.height = par->height;

    const AVCodec *dec = avcodec_find_decoder(par->codec_id);
    if (!dec) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Unsupported codec: %d\n", par->codec_id);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    save_ctx.dec_ctx = avcodec_alloc_context3(dec);
    if (!save_ctx.dec_ctx) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Failed to allocate codec context\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = avcodec_parameters_to_context(save_ctx.dec_ctx, par);
    if (ret < 0) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Failed to copy codec parameters\n");
        avcodec_free_context(&save_ctx.dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = avcodec_open2(save_ctx.dec_ctx, dec, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Failed to open codec: %s\n", errbuf);
        avcodec_free_context(&save_ctx.dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 初始化 SWS 和 RGB 缓冲区
    save_ctx.sws = sws_getContext(par->width, par->height, save_ctx.dec_ctx->pix_fmt,
                                  par->width, par->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!save_ctx.sws) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Failed to create SWS context\n");
        avcodec_free_context(&save_ctx.dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int rgb_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, par->width, par->height, 1);
    save_ctx.rgb_buf = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(rgb_size)));
    if (!save_ctx.rgb_buf) {
        sws_freeContext(save_ctx.sws);
        avcodec_free_context(&save_ctx.dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    save_ctx.rgb_frame = av_frame_alloc();
    if (!save_ctx.rgb_frame) {
        av_freep(&save_ctx.rgb_buf);
        sws_freeContext(save_ctx.sws);
        avcodec_free_context(&save_ctx.dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = av_image_fill_arrays(save_ctx.rgb_frame->data, save_ctx.rgb_frame->linesize, save_ctx.rgb_buf,
                               AV_PIX_FMT_RGB24, par->width, par->height, 1);
    if (ret < 0) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Failed to fill RGB arrays\n");
        av_frame_free(&save_ctx.rgb_frame);
        av_freep(&save_ctx.rgb_buf);
        avcodec_free_context(&save_ctx.dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    save_ctx.frame = av_frame_alloc();
    save_ctx.packet = av_packet_alloc();
    if (!save_ctx.packet || !save_ctx.frame) {
        fprintf(stderr, "[matroska_ffmpeg_save_frames] Failed to allocate packet/frame\n");
        freeDecoderAndSws(&save_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    while (save_ctx.frame_index < max_frames) {
        ret = av_read_frame(fmt_ctx, save_ctx.packet);
        if (ret < 0) {
            break;
        }

        if (save_ctx.packet->stream_index != video_stream_index) {
            av_packet_unref(save_ctx.packet);
            continue;
        }

        ret = avcodec_send_packet(save_ctx.dec_ctx, save_ctx.packet);
        if (ret < 0) {
            av_packet_unref(save_ctx.packet);
            continue;
        }

        while (save_ctx.frame_index < max_frames) {
            ret = avcodec_receive_frame(save_ctx.dec_ctx, save_ctx.frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                break;
            }
            decodeAndSaveOneFrame(&save_ctx);
        }

        av_packet_unref(save_ctx.packet);
    }

    freeDecoderAndSws(&save_ctx);
    av_packet_free(&save_ctx.packet);
    av_frame_free(&save_ctx.frame);
    avcodec_free_context(&save_ctx.dec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}

/* ---------- 可选：单独编译为可执行文件时的 main ---------- */
#define MATROSKA_FFMPEG_SAVE_FRAMES_MAIN
#ifdef MATROSKA_FFMPEG_SAVE_FRAMES_MAIN

/**
 * 使用 libavfilter 构建 HDR->SDR + BT.2020->BT.709 转换过滤图（方案 A）
 * 输入：解码后的 YUV 帧
 * 输出：RGB24（BT.709, full range），供 PNG 保存使用
 */
static int init_video_filter_graph(AVCodecContext *dec_ctx,
                                   AVStream *stream,
                                   AVFilterGraph **graph_out,
                                   AVFilterContext **buffersrc_ctx_out,
                                   AVFilterContext **buffersink_ctx_out) {
    if (!dec_ctx || !stream || !graph_out || !buffersrc_ctx_out || !buffersink_ctx_out) {
        return AVERROR(EINVAL);
    }

    int ret = 0;
    AVFilterGraph *graph = avfilter_graph_alloc();
    if (!graph) {
        return AVERROR(ENOMEM);
    }

    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    if (!buffersrc || !buffersink) {
        avfilter_graph_free(&graph);
        return AVERROR_FILTER_NOT_FOUND;
    }

    AVFilterContext *buffersrc_ctx = nullptr;
    AVFilterContext *buffersink_ctx = nullptr;

    char args[512];
    AVRational tb = stream->time_base;
    AVRational sar = stream->sample_aspect_ratio.num && stream->sample_aspect_ratio.den
                         ? stream->sample_aspect_ratio
                         : AVRational{1, 1};

    // 与 FFmpeg 文档一致的 buffer 参数格式
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
             tb.num, tb.den,
             sar.num, sar.den);

    if ((ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                            args, nullptr, graph)) < 0) {
        avfilter_graph_free(&graph);
        return ret;
    }

    if ((ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                            nullptr, nullptr, graph)) < 0) {
        avfilter_graph_free(&graph);
        return ret;
    }

    // 限定输出格式为 RGB24，方便后续直接写 PNG
    static const enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE};
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        avfilter_graph_free(&graph);
        return ret;
    }

    // 过滤链（使用 FFmpeg 自带的 tonemap + colorspace 滤镜，不依赖 zscale）
    // HDR -> SDR + BT.2020 -> BT.709 转换
    //
    // 滤镜链说明：
    //   1. colorspace: 将 PQ (smpte2084) + BT.2020 转换到线性光 + BT.2020
    //   2. tonemap: 在线性光下进行 HDR 到 SDR 的色调映射
    //   3. colorspace: 将线性光 + BT.2020 转换到 BT.709 gamma + BT.709
    //   4. format=rgb24: 输出 RGB24 格式
    //
    // 第一个 colorspace 参数（PQ -> Linear）：
    //   - 输入：从 frame metadata 自动获取（PQ + BT.2020）
    //   - trc=linear: 输出 transfer 为线性光
    //   - fast=1: 使用快速转换模式
    //
    // tonemap 参数：
    //   - tonemap=hable: 使用 Hable tone mapping 算法（类似 mpv 默认）
    //   - desat=0: 不降低饱和度
    //   - peak=100: 峰值亮度（nits）
    //
    // 第二个 colorspace 参数（Linear + BT.2020 -> BT.709）：
    //   - all=bt709: 输出所有参数都是 BT.709（包括 primaries, transfer, matrix）
    //   - fast=1: 使用快速转换模式
    const char *filter_descr =
            "colorspace=trc=linear:fast=1,"
            "tonemap=hable:desat=0:peak=100,"
            "colorspace=all=bt709:fast=1,"
            "format=rgb24";

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    if (!outputs || !inputs) {
        avfilter_inout_free(&outputs);
        avfilter_inout_free(&inputs);
        avfilter_graph_free(&graph);
        return AVERROR(ENOMEM);
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    if ((ret = avfilter_graph_parse_ptr(graph, filter_descr,
                                        &inputs, &outputs, nullptr)) < 0) {
        avfilter_inout_free(&outputs);
        avfilter_inout_free(&inputs);
        avfilter_graph_free(&graph);
        return ret;
    }

    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);

    if ((ret = avfilter_graph_config(graph, nullptr)) < 0) {
        avfilter_graph_free(&graph);
        return ret;
    }

    *graph_out = graph;
    *buffersrc_ctx_out = buffersrc_ctx;
    *buffersink_ctx_out = buffersink_ctx;
    return 0;
}

/**
 * 在指定时间点保存单帧图片
 */
int save_frame_at_time(const char *input_mkv, const char *output_path, int64_t time_seconds) {
    AVFormatContext *fmt_ctx = nullptr;
    // const AVInputFormat *ifmt = av_find_input_format("matroska");
    int ret = avformat_open_input(&fmt_ctx, input_mkv, nullptr, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        fprintf(stderr, "[save_frame_at_time] avformat_open_input failed: %s\n", errbuf);
        return -1;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "[save_frame_at_time] avformat_find_stream_info failed\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        fprintf(stderr, "[save_frame_at_time] no video stream found\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }
    int video_stream_index = ret;
    AVStream *stream = fmt_ctx->streams[video_stream_index];
    AVCodecParameters *par = stream->codecpar;

    // 计算时间戳（秒转时间基单位）
    AVRational time_base = stream->time_base;
    int64_t timestamp = av_rescale(time_seconds, time_base.den, time_base.num);

    fprintf(stderr, "[save_frame_at_time] Seeking to %lld seconds (timestamp: %lld, time_base: %d/%d)\n",
            (long long) time_seconds, (long long) timestamp, time_base.num, time_base.den);

    // 跳转到指定时间
    ret = av_seek_frame(fmt_ctx, video_stream_index, timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        fprintf(stderr, "[save_frame_at_time] av_seek_frame failed: %s\n", errbuf);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 初始化解码器
    const AVCodec *dec = avcodec_find_decoder(par->codec_id);
    if (!dec) {
        fprintf(stderr, "[save_frame_at_time] Unsupported codec: %d\n", par->codec_id);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecContext *dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx) {
        fprintf(stderr, "[save_frame_at_time] Failed to allocate codec context\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = avcodec_parameters_to_context(dec_ctx, par);
    if (ret < 0) {
        fprintf(stderr, "[save_frame_at_time] Failed to copy codec parameters\n");
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = avcodec_open2(dec_ctx, dec, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        fprintf(stderr, "[save_frame_at_time] Failed to open codec: %s\n", errbuf);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    avcodec_flush_buffers(dec_ctx);

    // 初始化过滤图（方案 A：优先使用 libavfilter + tonemap）
    AVFilterGraph *filter_graph = nullptr;
    AVFilterContext *buffersrc_ctx = nullptr;
    AVFilterContext *buffersink_ctx = nullptr;
    bool use_filter_graph = false;
    int filter_init_ret = init_video_filter_graph(dec_ctx, stream,
                                                  &filter_graph,
                                                  &buffersrc_ctx,
                                                  &buffersink_ctx);
    if (filter_init_ret < 0) {
        fprintf(stderr, "[save_frame_at_time] init_video_filter_graph failed (%d), fallback to sws_scale path.\n",
                filter_init_ret);
    } else {
        use_filter_graph = true;
    }

    // 若过滤图不可用，则退回到 sws_scale + 手动范围扩展路径
    SwsContext *sws = nullptr;
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    uint8_t *rgb_buf = nullptr;

    if (!frame || !rgb_frame || !packet) {
        fprintf(stderr, "[save_frame_at_time] Failed to allocate frame/packet\n");
        if (frame) av_frame_free(&frame);
        if (rgb_frame) av_frame_free(&rgb_frame);
        if (packet) av_packet_free(&packet);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int rgb_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, par->width, par->height, 1);
    rgb_buf = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(rgb_size)));
    if (!rgb_buf) {
        av_frame_free(&frame);
        av_frame_free(&rgb_frame);
        av_packet_free(&packet);
        avcodec_free_context(&dec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buf,
                         AV_PIX_FMT_RGB24, par->width, par->height, 1);

    // 读取并解码帧，直到找到关键帧或接近目标时间
    bool frame_saved = false;
    int frames_decoded = 0;
    int max_decode_attempts = 100; // 最多尝试解码100帧

    while (frames_decoded < max_decode_attempts && !frame_saved) {
        ret = av_read_frame(fmt_ctx, packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                fprintf(stderr, "[save_frame_at_time] End of file reached\n");
            } else {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                fprintf(stderr, "[save_frame_at_time] av_read_frame error: %s\n", errbuf);
            }
            break;
        }

        if (packet->stream_index != video_stream_index) {
            av_packet_unref(packet);
            continue;
        }

        ret = avcodec_send_packet(dec_ctx, packet);
        if (ret < 0) {
            av_packet_unref(packet);
            continue;
        }

        while (!frame_saved) {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                break;
            }

            frames_decoded++;

            // 检查帧时间是否接近目标时间（允许1秒误差）
            int64_t frame_pts_seconds = av_rescale(frame->pts, time_base.num, time_base.den);
            int64_t time_diff = llabs(frame_pts_seconds - time_seconds);

            if (time_diff <= 1 || frames_decoded == 1) {
                // 允许1秒误差，或第一帧
                if (use_filter_graph && filter_graph && buffersrc_ctx && buffersink_ctx) {
                    // 方案 A：通过 libavfilter 链做 HDR->SDR + BT.709 转换
                    AVFrame *filt_frame = av_frame_alloc();
                    if (!filt_frame) {
                        break;
                    }

                    // 把原始解码帧送入过滤图
                    ret = av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
                    if (ret < 0) {
                        av_frame_free(&filt_frame);
                        fprintf(stderr, "[save_frame_at_time] av_buffersrc_add_frame_flags failed: %d\n", ret);
                        break;
                    }

                    // 从过滤图取出已经做完 tone-mapping 的 RGB24 帧
                    ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                    if (ret < 0) {
                        av_frame_free(&filt_frame);
                        if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                            fprintf(stderr, "[save_frame_at_time] av_buffersink_get_frame failed: %d\n", ret);
                        }
                        break;
                    }

                    // 过滤图输出的像素格式应该是 RGB24（前面已经通过 pix_fmts 限定）
                    if (filt_frame->format != AV_PIX_FMT_RGB24) {
                        fprintf(stderr, "[save_frame_at_time] Unexpected filtered pixel format: %d\n",
                                filt_frame->format);
                        av_frame_free(&filt_frame);
                        break;
                    }

                    // 拷贝到我们自己的 RGB 缓冲区（rgb_frame/rgb_buf），便于统一调用 stbi_write_png
                    av_image_copy(rgb_frame->data, rgb_frame->linesize,
                                  (const uint8_t **) filt_frame->data, filt_frame->linesize,
                                  AV_PIX_FMT_RGB24, par->width, par->height);

                    av_frame_free(&filt_frame);

                    int ok = stbi_write_png(output_path, par->width, par->height, 3,
                                            rgb_frame->data[0], rgb_frame->linesize[0]);
                    if (ok) {
                        fprintf(
                            stderr,
                            "[save_frame_at_time] Successfully saved frame at %lld seconds to: %s (via libavfilter)\n",
                            (long long) time_seconds, output_path);
                        frame_saved = true;
                    } else {
                        fprintf(stderr, "[save_frame_at_time] Failed to save PNG: %s\n", output_path);
                    }
                } else {
                    // 回退路径：使用 sws_scale + 手动范围扩展（旧实现）
                    // 获取颜色空间信息（在循环外定义，以便后续使用）
                    AVColorSpace frame_colorspace = frame->colorspace;
                    if (frame_colorspace == AVCOL_SPC_UNSPECIFIED) frame_colorspace = AVCOL_SPC_BT709;

                    // 创建 SWS context（如果还没有）
                    if (!sws) {
                        AVPixelFormat frame_fmt = (AVPixelFormat) frame->format;
                        sws = sws_getContext(par->width, par->height, frame_fmt,
                                             par->width, par->height, AV_PIX_FMT_RGB24,
                                             SWS_BILINEAR | SWS_ACCURATE_RND | SWS_FULL_CHR_H_INT,
                                             nullptr, nullptr, nullptr);
                        if (!sws) {
                            fprintf(stderr, "[save_frame_at_time] Failed to create SWS context\n");
                            break;
                        }

                        // 设置颜色空间
                        int color_range_sw = frame->color_range;
                        if (color_range_sw == 0) color_range_sw = 1;

                        int srcRange_sw = (color_range_sw == 2) ? 1 : 0;
                        int dstRange_sw = 1;

                        int sws_colorspace = SWS_CS_DEFAULT;
                        if (frame_colorspace == AVCOL_SPC_BT709) {
                            sws_colorspace = SWS_CS_ITU709;
                        } else if (frame_colorspace == AVCOL_SPC_BT2020_NCL || frame_colorspace ==
                                   AVCOL_SPC_BT2020_CL) {
                            sws_colorspace = SWS_CS_BT2020;
                        } else if (frame_colorspace == AVCOL_SPC_BT470BG || frame_colorspace == AVCOL_SPC_SMPTE170M) {
                            sws_colorspace = SWS_CS_ITU601;
                        }

                        const int *inv_table = sws_getCoefficients(sws_colorspace);
                        const int *table = (frame_colorspace == AVCOL_SPC_BT2020_NCL || frame_colorspace ==
                                            AVCOL_SPC_BT2020_CL)
                                               ? sws_getCoefficients(SWS_CS_ITU709)
                                               : sws_getCoefficients(SWS_CS_DEFAULT);
                        sws_setColorspaceDetails(sws, inv_table, srcRange_sw, table, dstRange_sw, 0, 1 << 16, 1 << 16);
                    }

                    // 转换颜色空间
                    ret = sws_scale(sws, frame->data, frame->linesize, 0, par->height,
                                    rgb_frame->data, rgb_frame->linesize);
                    if (ret > 0) {
                        // 强制处理 limited range 到 full range 的扩展
                        int frame_color_range = frame->color_range;
                        if (frame_color_range == 0) frame_color_range = 1; // 默认 limited range

                        if (frame_color_range == 1) {
                            // limited range
                            uint8_t *rgb_data = rgb_frame->data[0];
                            int stride = rgb_frame->linesize[0];

                            for (int y = 0; y < par->height; y++) {
                                uint8_t *row = rgb_data + y * stride;
                                for (int x = 0; x < par->width; x++) {
                                    int r = row[x * 3];
                                    int g = row[x * 3 + 1];
                                    int b = row[x * 3 + 2];

                                    if (r <= 235) {
                                        r = (int) ((r - 16) * 255.0f / 219.0f + 0.5f);
                                    }
                                    if (g <= 235) {
                                        g = (int) ((g - 16) * 255.0f / 219.0f + 0.5f);
                                    }
                                    if (b <= 235) {
                                        b = (int) ((b - 16) * 255.0f / 219.0f + 0.5f);
                                    }

                                    row[x * 3] = (uint8_t) (r < 0 ? 0 : (r > 255 ? 255 : r));
                                    row[x * 3 + 1] = (uint8_t) (g < 0 ? 0 : (g > 255 ? 255 : g));
                                    row[x * 3 + 2] = (uint8_t) (b < 0 ? 0 : (b > 255 ? 255 : b));
                                }
                            }
                        }

                        int ok = stbi_write_png(output_path, par->width, par->height, 3,
                                                rgb_frame->data[0], rgb_frame->linesize[0]);
                        if (ok) {
                            fprintf(
                                stderr,
                                "[save_frame_at_time] Successfully saved frame at %lld seconds to: %s (via sws)\n",
                                (long long) time_seconds, output_path);
                            frame_saved = true;
                        } else {
                            fprintf(stderr, "[save_frame_at_time] Failed to save PNG: %s\n", output_path);
                        }
                    }
                }
                break;
            }
        }

        av_packet_unref(packet);
    }

    // 清理
    if (sws) sws_freeContext(sws);
    if (filter_graph) {
        avfilter_graph_free(&filter_graph);
        buffersrc_ctx = nullptr;
        buffersink_ctx = nullptr;
    }
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_packet_free(&packet);
    if (rgb_buf) av_freep(&rgb_buf);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);

    return frame_saved ? 0 : -1;
}

// int main(int argc, char *argv[]) {
//     const char *path = (argc > 1) ? argv[1] : "d:\\ff.mkv";
//
//     // 保存4张图片：30秒、60秒、120秒、180秒
//     int time_points[] = {30, 60, 120, 180};
//     int num_points = sizeof(time_points) / sizeof(time_points[0]);
//
//     fprintf(stderr, "[main] Saving %d frames at specified time points from: %s\n", num_points, path);
//     fflush(stderr);
//
//     for (int i = 0; i < num_points; i++) {
//         char output_path[512];
//         snprintf(output_path, sizeof(output_path), "frame_%03ds.png", time_points[i]);
//
//         fprintf(stderr, "\n[main] Processing time point %d: %d seconds -> %s\n", i + 1, time_points[i], output_path);
//         fflush(stderr);
//
//         int ret = save_frame_at_time(path, output_path, time_points[i]);
//         if (ret < 0) {
//             fprintf(stderr, "[main] Failed to save frame at %d seconds\n", time_points[i]);
//         }
//     }
//
//     fprintf(stderr, "\n[main] Completed saving all frames.\n");
//     return 0;
// }
#endif

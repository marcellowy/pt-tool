/**
 * @file matroska_ffmpeg_save_frames.h
 * @brief MKV 解封装 + FFmpeg 解码 + 保存帧为 PPM 图片（可供其他模块调用）
 */

#pragma once
#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * 从 MKV 文件中解封装视频轨，用 FFmpeg 解码并保存前 N 帧为 PPM 图片
 *
 * @param input_mkv    MKV 文件路径
 * @param output_prefix 输出图片文件名前缀，如 "frame" 则生成 frame_00000.ppm, frame_00001.ppm ...
 * @param max_frames   最多保存的帧数，≤0 时使用默认 30
 * @return 0 成功，非 0 失败（如文件打不开、无视频轨、解码失败等）
 */
int matroska_ffmpeg_save_frames(const char *input_mkv,
                                const char *output_prefix,
                                int max_frames);

int save_frame_at_time(const char *input_mkv,
                       const char *output_path,
                       int64_t time_seconds);
#ifdef __cplusplus
}
#endif

#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "av_ffmpeg.h"
#include "av_codec_stb_image_jpg.h"
#include "av_codec_jpg.h"
#include "av_codec_png.h"
#include  "matroska_ffmpeg_save_frames.h"

class FFmpegTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
			return;
		}

		std::tstring config_file = TEXT("config.toml");
		if (av::env::is_dev()) {
			config_file = TEXT("config_dev.toml");
		}
		if (!Config::instance().parse(config_file)) {
			loge("parse config.toml failed");
			return;
		}
	}

	void TearDown() override {
	}
};

TEST_F(FFmpegTest, test_ffmpeg) {
	const char *path = "d:\\ff.mkv";

	// 保存4张图片：30秒、60秒、120秒、180秒
	int time_points[] = {30, 60, 120, 180};
	int num_points = sizeof(time_points) / sizeof(time_points[0]);

	fprintf(stderr, "[main] Saving %d frames at specified time points from: %s\n", num_points, path);
	fflush(stderr);

	for (int i = 0; i < num_points; i++) {
		char output_path[512];
		snprintf(output_path, sizeof(output_path), "frame_%03ds.png", time_points[i]);

		fprintf(stderr, "\n[main] Processing time point %d: %d seconds -> %s\n", i + 1, time_points[i], output_path);
		fflush(stderr);

		int ret = save_frame_at_time(path, output_path, time_points[i]);
		if (ret < 0) {
			fprintf(stderr, "[main] Failed to save frame at %d seconds\n", time_points[i]);
		}
	}

	fprintf(stderr, "\n[main] Completed saving all frames.\n");
}

TEST_F(FFmpegTest, DISABLED_captureFrame_stb) {
	// 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 120
	const std::vector<int64_t> tt = {5, 10};
	int64_t count = 0;

	av::codec::StbPNG stbPng([&count](void *data, int size) {
		std::tstringstream oo;
		oo << TEXT("test_") << count << TEXT(".png");
		std::ofstream out_file(av::str::toA(oo.str()), std::ios::binary);
		out_file.write(static_cast<char *>(data), size); // 写入数据到文件
		count++;
	});

	if (!av::ffmpeg::captureFrame(TEXT("C:\\Users\\chadwang\\Videos\\2026-01-08_09-55-11.ts"), tt, stbPng)) {
		loge("captureFrame failed!!!");
	}
}


TEST_F(FFmpegTest, DISABLED_captureFrame_jpg) {
	const std::vector<int64_t> tt = {
		60, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 120
	};
	int64_t count = 0;

	av::codec::JPG jpg([&count](uint8_t *data, size_t size) {
		logi("capture freame callback, {}", count);
		// 
		std::string filename = fmt::format("frame_{}.jpg", count);
		FILE *f = fopen(filename.c_str(), "wb");
		if (f == NULL) {
			loge("open {} failed", av::str::toA(filename));
			return;
		}
		fwrite(data, 1, size, f);
		count++;
	});

	if (!av::ffmpeg::captureFrame(TEXT("/home/marcello/tmp/中.mp4"), tt, jpg)) {
		loge("captureFrame failed!!!");
	}
}

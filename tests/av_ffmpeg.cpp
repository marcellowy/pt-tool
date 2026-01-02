#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "src/config.h"
#include "av_ffmpeg.h"
#include "av_codec_stb_image_jpg.h"
#include "av_codec_jpg.h"
#include "av_codec_png.h"

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

TEST_F(FFmpegTest, DISABLED_captureFrame_stb) {

	const std::vector<int64_t> tt = { 60, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 120 };
	int64_t count = 0;

	av::codec::StbPNG stbPng([&count](void* data, int size) {
		std::tstringstream oo;
		oo << TEXT("test_") << count << TEXT(".png");
		std::ofstream out_file(av::str::toA(oo.str()), std::ios::binary);
		out_file.write(static_cast<char*>(data), size);  // 写入数据到文件
		count++;
	});

	if (!av::ffmpeg::captureFrame(TEXT("/home/marcello/tmp/中.mp4"), tt, stbPng)) {
		loge("captureFrame failed!!!");
	}
}


TEST_F(FFmpegTest, DISABLED_captureFrame_jpg) {

	const std::vector<int64_t> tt = { 60, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960, 120 };
	int64_t count = 0;

	av::codec::JPG jpg([&count](uint8_t* data, size_t size) {
		logi("capture freame callback, {}", count);
		// save
		char filename[128];
		sprintf(filename, "frame%lld.jpg", count);
		FILE* f = fopen(filename, "wb");
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

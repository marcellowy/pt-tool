#include "gtest/gtest.h"
#include "av_env.h"
#include "logger.h"
#include "av_queue.h"
#include "av_cron.h"
#include "av_async.h"
#include "av_log.h"
#include "av_base64.h"

class CronTest : public ::testing::Test {
protected:
	void SetUp() override {
		if (!Logger::instance().open()) {
			std::cout << "can not open log" << std::endl;
			return;
		}
	}

	void TearDown() override {
		
	}
};

TEST_F(CronTest, Cron) {

	Queue<int> q;
	av::cron::Cron a(TEXT("1"), TEXT("*/1 * * * * *"), [&q] {
		av::async::Exit exit_cron([&q] {
			logi("test aa end base64: {}", av::base64::encode("1234567890"));
			q.push(2);
			});
		q.push(1);
	});
	a.start();

	auto tmp_thread = std::thread([&a, &q] {
		std::this_thread::sleep_for(std::chrono::seconds(3));
		a.stop();
		q.close();
	});
	
	auto tmp_thread2 = std::thread([&q] {
		int a = 0;
		while (q.pop(a)) {
			logi("THREAD2 q val {}", a);
		}
		logi("q closed pop thread exit");
	});

	auto tmp_thread3 = std::thread([&q] {
		int a = 0;
		while (q.pop(a)) {
			logi("THREAD3 q val {}", a);
		}
		logi("q closed pop thread exit");
	});

	tmp_thread.join();
	tmp_thread2.join();
	tmp_thread3.join();

	return;
}

TEST_F(CronTest, Cron_task_repeate_in) {
	// 如果上一次任务未结束,下一次任务就来了，那么会阻塞,不会重复执行
	av::cron::Cron a(TEXT("1"), TEXT("*/1 * * * * *"), [] {
		logi("test aa end base64: {}", av::base64::encode("1234567890"));
		std::this_thread::sleep_for(std::chrono::seconds(2));
	});
	a.start();

	auto tmp_thread = std::thread([&a] {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		a.stop();
	});
	tmp_thread.join();
	return;
}

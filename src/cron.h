#ifndef TEST_CRON_H_
#define TEST_CRON_H_
#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <functional>
#include <condition_variable>
#include <mutex>

#include "croncpp/croncpp.h"
#include "av_string.h"

// <seconds> <minutes> <hours> <days of month> <months> <days of week> <years>

class Cron {
public:
	Cron(const std::tstring& name, const std::tstring& cronexpr, std::function<void()> task);
	~Cron();

	void start();
	void stop();
private:
	void run();

private:
	std::tstring name_;
	std::tstring cronexpr_;
	std::function<void()> task_;
	std::mutex mtx_;
	std::condition_variable cv_;
	std::thread worker_;
	std::atomic<bool> running_{ false };
};


#endif // !TEST_CRON_H_

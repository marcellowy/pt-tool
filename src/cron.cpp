#include "cron.h"
#include "av_log.h"

Cron::Cron(const std::tstring& name, const std::tstring& cronexpr, std::function<void()> task) :
	name_(name),
	cronexpr_(cronexpr),
	task_(std::move(task)) {
}

Cron::~Cron() {
	stop();
}

void Cron::start() {
	if (running_) return;
	logi("cron {} start...", av::str::toA(name_));
	running_ = true;
	worker_ = std::thread(&Cron::run, this);
}

void Cron::stop() {
	if (!running_) return;
	logi("cron {} stop", av::str::toA(name_));
	// 
	running_ = false;
	if (worker_.joinable()) {
		cv_.notify_one(); // wakup thread
		worker_.join();
	}
}

void Cron::run() {
	logi("cron {} thread start", av::str::toA(name_));
	cron::cronexpr expr;
	try {
		expr = cron::make_cron(av::str::toA(cronexpr_));
	}
	catch (cron::bad_cronexpr const& e) {
		loge("bad_cronexpr {}", e.what());
		return;
	}
	std::unique_lock<std::mutex> lock(mtx_);
	while (running_) {
		auto next = cron::cron_next(expr, std::chrono::system_clock::now());
		// next ok or running_ is false
		cv_.wait_until(lock, next, [this] { return !this->running_.load(); });
		if (!running_) break;
		task_();
	}
	logi("cron {} thread end", av::str::toA(name_));
}
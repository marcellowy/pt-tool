#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <vector>

template<typename F>
class TimedFunctions {
public:
    TimedFunctions() = default;

    ~TimedFunctions() {
        close(false);
    }

    TimedFunctions(const TimedFunctions &) = delete;
    TimedFunctions & operator=(const TimedFunctions &) = delete;

    bool open(const std::string & name = "") {
        std::lock_guard lifecycle_lock(lifecycle_mtx_);
        std::lock_guard lock(mtx_);
        if (thread_.joinable() || stopping_) {
            return false;
        }

        clearTasksLocked();
        clearTimersLocked();
        wait_ = false;

        try {
            thread_ = std::jthread([this](std::stop_token stoken) {
                run(std::move(stoken));
            });
        } catch (...) {
            accepting_ = false;
            stopping_ = false;
            return false;
        }

        accepting_ = true;
#ifdef _WIN32
        if (!name.empty()) {
            std::wstring wname(name.begin(), name.end());
            SetThreadDescription(thread_.native_handle(), wname.c_str());
        }
#endif
        return true;
    }

    void close(bool wait = false) {
        bool called_from_worker = false;

        std::unique_lock lifecycle_lock(lifecycle_mtx_);
        {
            std::unique_lock lock(mtx_);
            if (!thread_.joinable()) {
                if (worker_id_ == std::this_thread::get_id()) {
                    return;
                }
                cv_.wait(lock, [this] { return !stopping_; });
                return;
            }

            wait_ = wait;
            accepting_ = false;
            stopping_ = true;
            called_from_worker = worker_id_ == std::this_thread::get_id();

            clearTimersLocked();
            if (!wait_) {
                clearTasksLocked();
            }

            thread_.request_stop();
            if (called_from_worker) {
                thread_.detach();
            }
        }

        cv_.notify_one();

        if (called_from_worker) {
            return;
        }

        thread_.join();
        {
            std::lock_guard lock(mtx_);
            finishStoppedLocked();
        }
    }

    bool running() const {
        std::lock_guard lock(mtx_);
        return accepting_ && !stopping_;
    }

    void async(F func) {
        post(std::move(func));
    }

    bool sync(F func, int timeout_ms = 0) {
        if (isWorkerThread()) {
            func();
            return true;
        }

        auto p = std::make_shared<std::promise<void>>();
        auto f = p->get_future();
        auto shared_func = std::make_shared<F>(std::move(func));
        if (!post([shared_func, p]() {
            try {
                (*shared_func)();
                p->set_value();
            } catch (...) {
                p->set_exception(std::current_exception());
            }
        })) {
            return false;
        }
        p.reset();

        auto consume_result = [&f]() {
            try {
                f.get();
                return true;
            } catch (...) {
                return false;
            }
        };

        if (timeout_ms <= 0) {
            f.wait();
            return consume_result();
        }
        if (f.wait_for(std::chrono::milliseconds(timeout_ms)) != std::future_status::ready) {
            return false;
        }
        return consume_result();
    }

    int delayed(int ms, F func) {
        int id = next_id_.fetch_add(1, std::memory_order_relaxed);
        auto deadline = std::chrono::steady_clock::now()
            + std::chrono::milliseconds(ms);
        {
            std::lock_guard lock(mtx_);
            if (!accepting_ || stopping_) return -1;
            active_timers_.insert(id);
            timers_.push({deadline, std::move(func), id, 0});
        }
        cv_.notify_one();
        return id;
    }

    int periodic(int ms, F func) {
        if (ms <= 0) {
            return -1;
        }

        int id = next_id_.fetch_add(1, std::memory_order_relaxed);
        auto deadline = std::chrono::steady_clock::now()
            + std::chrono::milliseconds(ms);
        {
            std::lock_guard lock(mtx_);
            if (!accepting_ || stopping_) return -1;
            active_timers_.insert(id);
            timers_.push({deadline, std::move(func), id, ms});
        }
        cv_.notify_one();
        return id;
    }

    void cancel(int timer_id) {
        if (timer_id <= 0) {
            return;
        }

        {
            std::lock_guard lock(mtx_);
            if (active_timers_.erase(timer_id) == 0) {
                return;
            }
            cancelled_.insert(timer_id);
        }
        cv_.notify_one();
    }

private:
    struct TimerEntry {
        std::chrono::steady_clock::time_point deadline;
        F func;
        int id;
        int interval_ms; // 0 = one-shot, >0 = periodic

        bool operator>(const TimerEntry & other) const {
            return deadline > other.deadline;
        }
    };

    using TimerQueue = std::priority_queue<
        TimerEntry, std::vector<TimerEntry>, std::greater<TimerEntry>>;

    static constexpr int kReadyTimerBurstLimit = 64;

    mutable std::mutex mtx_;
    mutable std::mutex lifecycle_mtx_;
    std::condition_variable cv_;
    std::queue<F> tasks_;
    TimerQueue timers_;
    std::set<int> cancelled_;
    std::set<int> active_timers_;
    std::jthread thread_;
    std::thread::id worker_id_;
    bool accepting_ = false;
    bool stopping_ = false;
    bool wait_ = false;
    std::atomic<int> next_id_{1};

    bool post(F func) {
        {
            std::lock_guard lock(mtx_);
            if (!accepting_ || stopping_) {
                return false;
            }
            tasks_.push(std::move(func));
        }
        cv_.notify_one();
        return true;
    }

    bool isWorkerThread() const {
        std::lock_guard lock(mtx_);
        return worker_id_ == std::this_thread::get_id();
    }

    void clearTasksLocked() {
        tasks_ = std::queue<F>();
    }

    void clearTimersLocked() {
        timers_ = TimerQueue();
        cancelled_.clear();
        active_timers_.clear();
    }

    void finishStoppedLocked() {
        accepting_ = false;
        stopping_ = false;
        wait_ = false;
        worker_id_ = std::thread::id();
        clearTasksLocked();
        clearTimersLocked();
    }

    void discardCancelledTimersLocked() {
        while (!timers_.empty() && cancelled_.count(timers_.top().id) != 0) {
            const int id = timers_.top().id;
            cancelled_.erase(id);
            active_timers_.erase(id);
            timers_.pop();
        }
    }

    bool runOneTaskLocked(std::unique_lock<std::mutex> & lock) {
        if (tasks_.empty()) {
            return false;
        }

        auto func = std::move(tasks_.front());
        tasks_.pop();

        lock.unlock();
        func();
        lock.lock();
        return true;
    }

    bool runOneTimerLocked(std::unique_lock<std::mutex> & lock, std::stop_token & stoken) {
        discardCancelledTimersLocked();
        if (timers_.empty()) {
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        if (timers_.top().deadline > now) {
            return false;
        }

        auto entry = timers_.top();
        timers_.pop();
        const bool periodic = entry.interval_ms > 0;
        if (!periodic) {
            active_timers_.erase(entry.id);
        }

        lock.unlock();
        entry.func();
        lock.lock();

        if (!periodic) {
            return true;
        }

        if (stoken.stop_requested() || stopping_) {
            active_timers_.erase(entry.id);
            cancelled_.erase(entry.id);
            return true;
        }

        if (cancelled_.erase(entry.id) != 0 || active_timers_.count(entry.id) == 0) {
            active_timers_.erase(entry.id);
            return true;
        }

        entry.deadline = std::chrono::steady_clock::now()
            + std::chrono::milliseconds(entry.interval_ms);
        timers_.push(std::move(entry));
        return true;
    }

    void run(std::stop_token stoken) {
        std::stop_callback on_stop(stoken, [this] { cv_.notify_one(); });
        std::unique_lock lock(mtx_);
        worker_id_ = std::this_thread::get_id();
        int ready_timer_burst = 0;

        while (!stoken.stop_requested()) {
            discardCancelledTimersLocked();

            // 1. Run at most one ready timer before giving immediate work a chance.
            if (runOneTimerLocked(lock, stoken)) {
                ++ready_timer_burst;
                if (!tasks_.empty()) {
                    ready_timer_burst = 0;
                    runOneTaskLocked(lock);
                } else if (ready_timer_burst >= kReadyTimerBurstLimit) {
                    ready_timer_burst = 0;
                    discardCancelledTimersLocked();
                    const auto now = std::chrono::steady_clock::now();
                    if (!timers_.empty() && timers_.top().deadline <= now) {
                        cv_.wait_for(lock, std::chrono::milliseconds(1), [this, &stoken] {
                            return !tasks_.empty() || stoken.stop_requested() || stopping_;
                        });
                    }
                }
                continue;
            }
            ready_timer_burst = 0;

            // 2. Process one immediate task.
            if (!tasks_.empty()) {
                runOneTaskLocked(lock);
                continue;
            }

            // 3. Sleep until next event (zero CPU when idle)
            if (timers_.empty()) {
                cv_.wait(lock, [this, &stoken] {
                    return !tasks_.empty() || stoken.stop_requested()
                        || stopping_ || !timers_.empty();
                });
            } else {
                const auto deadline = timers_.top().deadline;
                cv_.wait_until(lock, deadline, [this, &stoken, deadline] {
                    return !tasks_.empty()
                        || stoken.stop_requested()
                        || stopping_
                        || timers_.empty()
                        || timers_.top().deadline < deadline
                        || cancelled_.count(timers_.top().id) != 0;
                });
            }
        }

        // Drain remaining immediate tasks if requested
        if (wait_) {
            while (!tasks_.empty()) {
                runOneTaskLocked(lock);
            }
        }

        finishStoppedLocked();
        lock.unlock();
        cv_.notify_all();
    }
};

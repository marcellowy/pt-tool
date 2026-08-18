#pragma once

#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <functional>
#include <mutex>
#include <utility>

namespace av::worker {

class Worker {
public:
    Worker();
    ~Worker();

    bool open(const std::string & name = "");
    void close(bool wait = false);
    void async(std::function<void()> func);
    bool sync(std::function<void()> func, int timeout = 0);
    bool running();

    int  delayed(int ms, std::function<void()> func);
    int  periodic(int ms, std::function<void()> func);
    void cancel(int timer_id);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}

namespace av::worker {
Worker & global_worker();
void shutdown_global_worker();

class CancellableTimer {
public:
    CancellableTimer() = default;
    ~CancellableTimer() { cancel(); }

    CancellableTimer(const CancellableTimer &) = delete;
    CancellableTimer & operator=(const CancellableTimer &) = delete;

    // Schedule fn to run after ms milliseconds on the global worker thread.
    // Any previously-pending invocation is cancelled.
    void schedule(int ms, std::function<void()> fn) {
        cancel();

        auto token = std::make_shared<std::atomic<bool>>(false);
        {
            std::lock_guard<std::mutex> lk(mu_);
            token_ = token;
        }

        auto wrapped = std::make_shared<std::function<void()>>(std::move(fn));
        int id = global_worker().delayed(ms, [token, wrapped]() {
            if (!token->load(std::memory_order_acquire)) {
                (*wrapped)();
            }
        });

        std::lock_guard<std::mutex> lk(mu_);
        if (token_ == token) {
            timer_id_ = id;
        }
    }

    void cancel() {
        std::shared_ptr<std::atomic<bool>> tok;
        int id = 0;
        {
            std::lock_guard<std::mutex> lk(mu_);
            tok = std::move(token_);
            token_.reset();
            id = timer_id_;
            timer_id_ = 0;
        }
        if (tok) {
            tok->store(true, std::memory_order_release);
        }
        if (id != 0) {
            global_worker().cancel(id);
        }
    }

private:
    std::mutex mu_;
    std::shared_ptr<std::atomic<bool>> token_;
    int timer_id_{0};
};
}

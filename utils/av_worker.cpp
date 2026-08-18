#include "av_timed_functions.h"
#include "av_worker.h"
namespace av::worker {
struct Worker::Impl {
    TimedFunctions<std::function<void()>> functions;
};

Worker::Worker() : impl_(std::make_unique<Impl>()) {}

Worker::~Worker() = default;

bool Worker::open(const std::string & name) {
    return impl_->functions.open(name);
}

void Worker::close(bool wait) {
    impl_->functions.close(wait);
}

void Worker::async(std::function<void()> func) {
    impl_->functions.async(std::move(func));
}

bool Worker::sync(std::function<void()> func, int timeout) {
    return impl_->functions.sync(std::move(func), timeout);
}

bool Worker::running() {
    return impl_->functions.running();
}

int Worker::delayed(int ms, std::function<void()> func) {
    return impl_->functions.delayed(ms, std::move(func));
}

int Worker::periodic(int ms, std::function<void()> func) {
    return impl_->functions.periodic(ms, std::move(func));
}

void Worker::cancel(int timer_id) {
    impl_->functions.cancel(timer_id);
}
}

namespace av::worker {
Worker & instance() {
    static Worker w;
    static std::once_flag once;
    std::call_once(once, [] {
        std::string thread_name = "global_worker";
        w.open(thread_name);
    });
    return w;
}

Worker & global_worker() {
    return instance();
}

void shutdown_global_worker() {
    // Drain pending immediate tasks then join the worker thread.
    instance().close(true);
}
} // namespace

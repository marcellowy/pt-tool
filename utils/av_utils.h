#ifndef AV_UTILS_H
#define AV_UTILS_H

#include <thread>
#include <optional>
#include <chrono>

namespace av::utils {

    enum class RetryBackoffPolicy {
        FixedInterval,      // 固定间隔
        ExponentialBackoff  // 指数退避
    };

    template <typename Func>
    auto retry_backoff(
        Func&& func,
        int max_retries,
        int64_t base_delay_ms = 1000,
        RetryBackoffPolicy policy = RetryBackoffPolicy::FixedInterval
    ) -> std::optional<decltype(func())>
    {
        for (int attempt = 1; attempt <= max_retries; ++attempt) {
            try {
                return func();
            } catch (...) {
                if (attempt == max_retries) break;
                if (policy == RetryBackoffPolicy::ExponentialBackoff) {
                    auto delay = base_delay_ms * (1 << (attempt - 1));
                    logi("delay {}ms", delay);
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                } else if (policy == RetryBackoffPolicy::FixedInterval) {
                    logi("delay {}ms", base_delay_ms);
                    std::this_thread::sleep_for(std::chrono::milliseconds(base_delay_ms));
                }
            }
        }
        return std::nullopt;
    }
}

#endif //AV_UTILS_H

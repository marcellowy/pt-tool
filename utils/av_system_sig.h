#ifndef AV_SYSTEM_SIG_H_
#define AV_SYSTEM_SIG_H_

#include <atomic>
#include <functional>

namespace av {
    namespace system {
        std::atomic<bool> stop_{ false };
        void sig(std::function<void()> func);
    }
}

#endif
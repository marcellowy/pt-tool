#ifndef AV_ASYNC_H_
#define AV_ASYNC_H_

#include <functional>

namespace av {
    namespace async {
        class Exit {
        public:
            explicit Exit(std::function<void()> func)
                : func_(std::move(func)) {
            }

            ~Exit() {
                if (func_) func_();
            }

            // diable copy
            Exit(const Exit&) = delete;
            Exit& operator=(const Exit&) = delete;

            // allow move
            Exit(Exit&& other) noexcept
                : func_(std::move(other.func_)) {
                other.func_ = nullptr;
            }

            Exit& operator=(Exit&& other) noexcept {
                if (this != &other) {
                    func_ = std::move(other.func_);
                    other.func_ = nullptr;
                }
                return *this;
            }

        private:
            std::function<void()> func_;
        };
    }
}
#endif
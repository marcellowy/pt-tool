#include "av_system_sig.h"

void av::system::sig(std::function<void()> func) {
    auto tmp = std::move(func);
}

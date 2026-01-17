#ifndef AV_DOUBAN_IMG_H
#define AV_DOUBAN_IMG_H

#include <functional>
#include "av_string.h"

namespace av::douban {
    bool get_img(const std::tstring& url, const std::function<void(const char* buff, size_t size)>& cb);
}

#endif //AV_DOUBAN_IMG_H

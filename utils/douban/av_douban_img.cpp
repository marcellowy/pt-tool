#include "av_douban_img.h"
#include "av_http_v2.h"
#include "av_log.h"

namespace av::douban {
    bool get_img(const std::tstring& url, const std::function<void(const char* buff, size_t size)>& cb) {
        av::http_v2::Client cli;
        http_v2::Header header;
        header.kv["referer"] = "https://movie.douban.com/";
        header.kv["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/144.0.0.0 Safari/537.36";

        const auto resp = cli.get(av::str::toA(url), header);
        if (!resp) {
            logw("get {} failed", av::str::toA(url));
            return false;
        }
        if ( resp->status != 200) {
            logw("get {} failed,status {}", av::str::toA(url), resp->status);
            return false;
        }
        cb(resp->body.c_str(), resp->body.size());
        return true;
    }
}
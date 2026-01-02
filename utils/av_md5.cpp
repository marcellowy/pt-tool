#include "av_md5.h"

#include "openssl/md5.h"

#include "av_log.h"

namespace av {
	namespace hash {
		bool md5(const std::string& in_str, std::string& out_str) {
            unsigned char md5[MD5_DIGEST_LENGTH];
            MD5(reinterpret_cast<const unsigned char*>(in_str.c_str()), in_str.size(), md5);
            out_str = "";
            const char map[] = "0123456789abcdef";
            for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i) {
                out_str += map[md5[i] / 16];
                out_str += map[md5[i] % 16];
            }
			return true;
		}
	}
}

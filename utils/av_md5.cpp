#include "av_md5.h"

#include "openssl/md5.h"
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

#include "av_log.h"

namespace av {
	namespace hash {
		bool md5(const std::string& in_str, std::string& out_str) {
            unsigned char hash[EVP_MAX_MD_SIZE];
            unsigned int length = 0;

            // 1. 创建并初始化上下文
            EVP_MD_CTX* context = EVP_MD_CTX_new();

            // 2. 指定 MD5 算法
            if (EVP_DigestInit_ex(context, EVP_md5(), nullptr)) {
                // 3. 传入数据
                if (EVP_DigestUpdate(context, in_str.c_str(), in_str.length())) {
                    // 4. 获取计算结果
                    EVP_DigestFinal_ex(context, hash, &length);
                }
            }

            // 5. 释放上下文
            EVP_MD_CTX_free(context);

            // 6. 转换为 32 位小写十六进制字符串
            std::stringstream ss;
            for (unsigned int i = 0; i < length; ++i) {
                // setfill('0') 和 setw(2) 确保每个字节占两位（如 0a 而不是 a）
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }

            out_str = ss.str();
            return true;
		}
	}
}

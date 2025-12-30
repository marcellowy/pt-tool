#ifndef AV_BASE64_H_
#define AV_BASE64_H_

#include <string>
#include <vector>
#include <stdexcept>

namespace av {
    namespace base64 {

        static const char kEncodeTable[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        static const unsigned char kDecodeTable[256] = {
            /* ASCII table mapping */
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
            52,53,54,55,56,57,58,59,60,61,64,64,64, 0,64,64,
            64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
            64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
            // rest are 64
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
            64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
        };

        inline std::string encode(const unsigned char* data, size_t len) {
            std::string out;
            out.reserve(((len + 2) / 3) * 4);

            for (size_t i = 0; i < len; i += 3) {
                unsigned int v = data[i] << 16;
                if (i + 1 < len) v |= data[i + 1] << 8;
                if (i + 2 < len) v |= data[i + 2];

                out.push_back(kEncodeTable[(v >> 18) & 0x3F]);
                out.push_back(kEncodeTable[(v >> 12) & 0x3F]);
                out.push_back((i + 1 < len) ? kEncodeTable[(v >> 6) & 0x3F] : '=');
                out.push_back((i + 2 < len) ? kEncodeTable[v & 0x3F] : '=');
            }
            return out;
        }

        inline std::string encode(const std::string& s) {
            return encode(reinterpret_cast<const unsigned char*>(s.data()), s.size());
        }

        inline std::string decode(const std::string& input) {
            std::vector<unsigned char> out;
            size_t len = input.size();
            if (len % 4 != 0) {
                throw std::runtime_error("Invalid base64 length");
            }

            for (size_t i = 0; i < len; i += 4) {
                unsigned int v = 0;
                int pad = 0;

                for (int j = 0; j < 4; ++j) {
                    unsigned char c = input[i + j];
                    if (c == '=') {
                        v <<= 6;
                        pad++;
                    }
                    else {
                        unsigned char d = kDecodeTable[c];
                        if (d == 64) {
                            throw std::runtime_error("Invalid base64 character");
                        }
                        v = (v << 6) | d;
                    }
                }

                out.push_back((v >> 16) & 0xFF);
                if (pad < 2) out.push_back((v >> 8) & 0xFF);
                if (pad < 1) out.push_back(v & 0xFF);
            }

            return std::string(out.begin(), out.end());
        }

    } // namespace base64
} // namespace av

#endif
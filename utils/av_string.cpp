#include <string>
#include <codecvt>
#include <locale>

#include "av_string.h"
#include "av_log.h"

void av::str::replace(std::tstring& str, const std::tstring& old_str, const std::tstring& new_str) {
	size_t pos = str.find(old_str);
	if (pos != std::string::npos) {
		str.replace(pos, old_str.size(), new_str);
	}
}

void av::str::remove_suffix(std::tstring& s, const std::tstring& suffix) {
	if (s.ends_with(suffix)) {
		s.erase(s.size() - suffix.size());
	}
}

namespace av {
	namespace str {
		std::string toA(const std::string& str) {
			return str;
		}
		std::string toA(const std::wstring& str) {
			if (str.empty()) {
				return "";
			}
#ifdef _WIN32
            std::string mbs;
            mbs.reserve((str.length() + 1) * 2);
            int mbs_len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &mbs[0], static_cast<int>(mbs.capacity()), nullptr, nullptr);
            if (mbs_len < 1) {
                DWORD err = GetLastError();
                loge("WideCharToMultiByte failed with {}, last error {}", mbs_len, err);
                return "";
            }

            return std::string(&mbs[0], static_cast<size_t>(mbs_len));
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            return conv.to_bytes(str);
#endif // _WIN32
		}

		std::wstring toW(const std::string& str) {
			if (str.empty()) {
				return L"";
			}
#ifdef _WIN32
            std::wstring wcs;
            wcs.reserve(str.length() + 1);
            int wcs_len = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.length()), &wcs[0], static_cast<int>(wcs.capacity()));
            if (wcs_len < 1) {
                DWORD err = GetLastError();
                loge("MultiByteToWideChar failed with {}, last error {}", wcs_len, err);
                return L"";
            }

            return std::wstring(&wcs[0], static_cast<size_t>(wcs_len));
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            return conv.from_bytes(str);
#endif // _WIN32


		}

        std::tstring toT(const std::string& str) {
#if defined(UNICODE) || defined(_UNICODE)
            return toW(str);
#else
            return toA(str);
#endif
        }

        std::tstring toT(const std::wstring& str) {
#if defined(UNICODE) || defined(_UNICODE)
            return toW(str);
#else
            return toA(str);
#endif
        }

        std::string toUtf8(const std::string& str) {
            if (str.empty()) {
                return "";
            }
#ifdef _WIN32
            std::wstring wcs;
            wcs.reserve(str.length() + 1);
            int wcs_len = MultiByteToWideChar(CP_ACP, 0, &str[0], static_cast<int>(str.length()), &wcs[0], static_cast<int>(wcs.capacity()));
            if (wcs_len < 1) {
                DWORD err = GetLastError();
                loge("MultiByteToWideChar failed with {}, last error {}", wcs_len, err);
                return "";
            }
            std::string mbs;
            mbs.reserve(static_cast<size_t>(wcs_len) * 3 + 3);
            int mbs_len = WideCharToMultiByte(CP_UTF8, 0, &wcs[0], static_cast<int>(wcs_len), &mbs[0], static_cast<int>(mbs.capacity()), nullptr, nullptr);
            if (mbs_len < 1) {
                DWORD err = GetLastError();
                loge("WideCharToMultiByte failed with {}, last error {}", wcs_len, err);
                return "";
            }

            return std::string(&mbs[0], static_cast<size_t>(mbs_len));
#else
            return str;
#endif // _WIN32

        }

        std::string toUtf8(const std::wstring& str) {
            if (str.empty()) {
                return "";
            }
#ifdef _WIN32
            std::string mbs;
            mbs.reserve(str.length() * 3 + 3);
            int mbs_len = WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.length()), &mbs[0], static_cast<int>(mbs.capacity()), nullptr, nullptr);
            if (mbs_len < 1) {
                DWORD err = GetLastError();
                loge("WideCharToMultiByte failed with {}, last error {}", str.length(), err);
                return "";
            }

            return std::string(&mbs[0], static_cast<size_t>(mbs_len));
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            return conv.to_bytes(str);
#endif // _WIN32

        }

        std::string toUnicodeA(const std::string& str) {
            if (str.empty()) {
                return "";
            }

            auto mbs = toA(toUnicodeW(str));
            if (mbs.empty()) {
                logw("ToString failed");
                return "";
            }

            return mbs;
        }

        std::wstring toUnicodeW(const std::string& str) {
            if (str.empty()) {
                return L"";
            }
#ifdef _WIN32
            std::wstring wcs;
            wcs.reserve(str.length() + 1);
            int wcs_len = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.length()), &wcs[0], static_cast<int>(wcs.capacity()));
            if (wcs_len < 1) {
                DWORD err = GetLastError();
                loge("MultiByteToWideChar failed with {}, last error {}", wcs_len, err);
                return L"";
            }

            return std::wstring(&wcs[0], static_cast<size_t>(wcs_len));
#else
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            return conv.from_bytes(str);
#endif // _WIN32


        }

		std::wstring toW(const std::wstring& str) {
			return str;
		}

        std::string urlEncode(const std::string& in) {
            // 
            auto* ptr_in = &in[0];
            auto* ptr_in_end = ptr_in + in.length();
            std::string out((in.length() * 3) + 1, '\0');
            auto* ptr_out_begin = reinterpret_cast<uint8_t*>(&out[0]);
            auto* ptr_out = ptr_out_begin;

            const char* hex = "0123456789ABCDEF";
            while (ptr_in < ptr_in_end) {
                char c = *ptr_in++;
                if ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    c == '-' || c == '_' || c == '.' || c == '~') {
                    *ptr_out++ = c;
                }
                else {
                    ptr_out[0] = '%';
                    ptr_out[1] = hex[static_cast<uint8_t>(c) >> 4];
                    ptr_out[2] = hex[static_cast<uint8_t>(c) & 0x0f];
                    ptr_out += 3;
                }
            }

            // 
            *ptr_out = '\0';

            return std::string(reinterpret_cast<char*>(ptr_out_begin), ptr_out - ptr_out_begin);
        }

        std::string urlDecode(const std::string& in) {
            auto* ptr_in = &in[0];
            auto* ptr_in_end = ptr_in + in.length();
            std::string out;
            out.reserve(in.length() + 1);
            auto* ptr_out_begin = &out[0];
            auto* ptr_out = ptr_out_begin;

            // 
            while (ptr_in < ptr_in_end) {
                if (*ptr_in == '+') {
                    *ptr_out++ = ' ';
                    ++ptr_in;
                }
                else if (*ptr_in == '%' && ptr_in_end - ptr_in > 1 &&
                    isxdigit(static_cast<int>(ptr_in[1])) && isxdigit(static_cast<int>(ptr_in[2]))) {
                    auto c = ptr_in[1];
                    if (isupper(c))
                        c = tolower(c);
                    *ptr_out = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) << 4;
                    c = ptr_in[2];
                    if (isupper(c))
                        c = tolower(c);
                    *ptr_out += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
                    ptr_in += 3;
                    ++ptr_out;
                }
                else {
                    *ptr_out++ = *ptr_in++;
                }
            }

            // 
            *ptr_out = '\0';

            return std::string(ptr_out_begin, ptr_out - ptr_out_begin);
        }

        std::string toUpper(const std::string& lower) {
            std::string upper;
            upper.resize(lower.size());
            std::transform(lower.begin(), lower.end(), upper.begin(), [](const char c) {
                return std::toupper(c);
                });
            return upper;
        }

        std::string toLower(const std::string& upper) {
            std::string lower;
            lower.resize(upper.size());
            std::transform(upper.begin(), upper.end(), lower.begin(), [](const char c) {
                return std::tolower(c);
                });
            return lower;
        }

        std::tstring trim(const std::tstring& s) {
            return av::str::toT(trim(av::str::toA(s)));
        }

        std::string trim(const std::string& s) {
            std::string tmp = s;
            tmp.erase(0, tmp.find_first_not_of(" "));
            tmp.erase(tmp.find_last_not_of(" ") + 1);
            return tmp;
        }
	}
}

#include <cstdlib>

#include "av_env.h"
#include "av_log.h"

namespace av {
	namespace env {
		static std::tstring get_pt_tool_env();

		bool is_dev() {
			return get_pt_tool_env() == TEXT("dev");
		}

		std::tstring get(const std::tstring& key) {
			logi("get env {}", av::str::toA(key));
			const char* v = std::getenv(av::str::toA(key).c_str());
			if (v == NULL) {
				logi("no env {}", av::str::toA(key));
				return TEXT("");
			}
			logi("get env {} value {}", av::str::toA(key), av::str::toA(v));
			return av::str::toT(v);
		}

		std::tstring get_pt_tool_env() {
			return get(TEXT("PT_TOOL_ENV"));
		}
	}
}

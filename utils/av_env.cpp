#include "av_env.h"

namespace av {
	namespace env {
		static std::tstring get_pt_tool_env();

		bool is_dev() {
			return get_pt_tool_env() == TEXT("dev");
		}

		std::tstring get(const std::tstring& key) {
			const char* v = std::getenv(av::str::toA(key).c_str());
			if (v == NULL) {
				return TEXT("");
			}
			return av::str::toT(v);
		}

		std::tstring get_pt_tool_env() {
			return get(TEXT("PT_TOOL_ENV"));
		}
	}
}
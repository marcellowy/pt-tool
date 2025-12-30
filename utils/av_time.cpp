#include "av_time.h"

namespace av {
	namespace time {

		std::chrono::system_clock::time_point now() {
			return std::chrono::system_clock::now();
		}

		int64_t seconds() {
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now().time_since_epoch());
			return seconds.count();
		}

		int64_t milliseconds() {
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now().time_since_epoch());
			return milliseconds.count();
		}

		int64_t microseconds() {
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now().time_since_epoch());
			return microseconds.count();
		}
	}
}


#include <sstream>
#include <ctime>
#include <format>

#include "av_time.h"
#include "av_log.h"

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
		
		bool diff_now(const std::string& date_time, std::chrono::system_clock::duration& duration) {
			auto now = std::chrono::system_clock::now();

			// format input
			std::istringstream iss(date_time);
			std::tm tm = {};
			iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
			if (iss.fail()) {
				logw("format {} failed", date_time);
				return false;
			}
			
			// 
			std::time_t time = std::mktime(&tm);
			auto tp = std::chrono::system_clock::from_time_t(time);
			
			// tiemzone
			auto time_zone = std::chrono::current_zone();

			//
			auto zoned_tp = std::chrono::zoned_time{ time_zone, tp };
			auto zoned_now = std::chrono::zoned_time{ time_zone, now };

			logi("current time: {}", std::format("{:%Y-%m-%d %H:%M:%S}", zoned_now));
			logi("last time: {}", std::format("{:%Y-%m-%d %H:%M:%S}", zoned_tp));

			if(zoned_tp.get_sys_time() > zoned_now.get_sys_time()) {
				logw("{} greater than the current time", date_time);
				return false;
			}

			// compare
			duration = zoned_now.get_sys_time() - zoned_tp.get_sys_time();
			return true;
		}
	}
}


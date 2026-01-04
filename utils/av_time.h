#ifndef AV_TIME_H_
#define AV_TIME_H_

#include <iostream>
#include <chrono>

namespace av {
	namespace time {
		std::chrono::system_clock::time_point now();

		int64_t seconds();

		int64_t milliseconds();

		int64_t microseconds();

		// 使用当前系统时区, 计算时间过去了多少秒
		// 时间格式: %Y-%m-%d %H:%M:%S
		bool diff_now(const std::string& date_time, std::chrono::system_clock::duration& duration);
			
	}
}


#endif

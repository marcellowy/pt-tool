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
	}
}


#endif
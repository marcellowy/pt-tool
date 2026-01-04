#include <sstream>
#include <ctime>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time/local_time.hpp"

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
		
		bool diff_now(const std::string& date_time, int64_t& seconds) {
            using namespace boost::posix_time;
            using namespace boost::local_time;

            // 创建一个时区对象（假设使用 UTC）
            time_zone_ptr zone(new posix_time_zone("UTC+8"));

            // 解析给定时间字符串
            ptime past_time(boost::posix_time::time_from_string(date_time));

            // 获取当前时间
            ptime current_time = second_clock::universal_time();

            // 将当前时间转换为当地时区时间
            local_date_time current_local_time(current_time, zone);

            // 输出比较的时间
			//std::cout << "Past time: " << past_time << std::endl;
            //std::cout << "Current local time: " << current_local_time << std::endl;

			ptime local_ptime = current_local_time.local_time();
			//std::cout << "aa: " << local_ptime << std::endl;
            // 计算两个时间点的差值
            time_duration diff = local_ptime - past_time;

            // 输出时间差，单位是秒
            // std::cout << "Time difference in seconds: " << diff.total_seconds() << " seconds." << std::endl;
			seconds = diff.total_seconds();
			return true;
		}
	}
}


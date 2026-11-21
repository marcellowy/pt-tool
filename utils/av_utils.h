#ifndef AV_UTILS_H
#define AV_UTILS_H

#include <thread>
#include <optional>
#include <string>
#include <algorithm>
#include <locale>
#include "av_log.h"
#include "av_string.h"

namespace av::utils {

	//
	std::string formatBytes(std::uint64_t bytes);

	template <typename CharT>
	bool iequals(const std::basic_string<CharT>& a, const std::basic_string<CharT>& b) {
		if (a.size() != b.size()) return false;
		const auto& ct = std::use_facet<std::ctype<CharT>>(std::locale());
		return std::equal(a.begin(), a.end(), b.begin(),
			[&ct](CharT c1, CharT c2) {
				return ct.tolower(c1) == ct.tolower(c2);
			});
	}

	//
	enum class RetryBackoffPolicy {
		FixedInterval,      // 固定间隔
		ExponentialBackoff  // 指数退避
	};

	template <typename Func>
	auto retry_backoff(
		Func&& func,
		int max_retries,
		int64_t base_delay_ms = 1000,
		RetryBackoffPolicy policy = RetryBackoffPolicy::FixedInterval
	) -> std::optional<decltype(func())>
	{
		for (int attempt = 1; attempt <= max_retries; ++attempt) {
			try {
				return func();
			}
			catch (...) {
				if (attempt == max_retries) break;
				if (policy == RetryBackoffPolicy::ExponentialBackoff) {
					auto delay = base_delay_ms * (1 << (attempt - 1));
					logi("delay {}ms", delay);
					std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				}
				else if (policy == RetryBackoffPolicy::FixedInterval) {
					logi("delay {}ms", base_delay_ms);
					std::this_thread::sleep_for(std::chrono::milliseconds(base_delay_ms));
				}
			}
		}
		return std::nullopt;
	}
}

#endif //AV_UTILS_H

#include "av_utils.h"
#include <fmt/format.h>

namespace av::utils {
	std::string formatBytes(std::uint64_t bytes) {
		static const char* units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB" };
		if (bytes < 1024) return std::format("{} B", bytes);

		double size = static_cast<double>(bytes);
		int idx = 0;
		while (size >= 1024.0 && idx < 6) { size /= 1024.0; ++idx; }
		if (idx < 6 && std::round(size * 100.0) / 100.0 >= 1024.0) { size /= 1024.0; ++idx; }

		return std::format("{:.2f} {}", size, units[idx]);
	};
};

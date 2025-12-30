#ifndef MTEAM_CATEGORY_H_
#define MTEAM_CATEGORY_H_

#include "av_base.h"

#include <string_view>
#include <optional>
#include <string>
#include "av_string.h"

namespace mteam {
	namespace category {
		
		enum class Id {
			Unknown = 0,
			TVSeries = 402,
			Discover = 404,
			Sport = 407,
			Movie = 419,
		};

		std::tstring to_string(Id id);

		Id from_string(const std::tstring& str);
	}
};

#endif
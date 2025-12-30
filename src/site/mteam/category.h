#ifndef MTEAM_CATEGORY_H_
#define MTEAM_CATEGORY_H_

#include "av_base.h"

#include <string_view>
#include <optional>
#include <string>
#include "av_string.h"
#include "av_media_info.h"

namespace mteam {

	enum class CategoryId {
		Unknown = 0,
		TVSeries = 402,
		Discover = 404,
		Sport = 407,
		Movie = 419,
	};

	class Category {
	public:
		Category() = default;
		Category(const av::media::SourceCategory category);
		~Category();

		mteam::CategoryId getid();

	private:
		av::media::SourceCategory m_category;
	};
};

#endif
#include "category.h"

namespace mteam {
	Category::Category(const av::media::SourceCategory category) : m_category(category) {
	}

	Category::~Category() {
	}

	void Category::setSourceCategory(const av::media::SourceCategory category) {
		m_category = category;
	}


	mteam::CategoryId Category::getid() {
		switch (m_category) {
			case av::media::SourceCategory::Unknown:
				return CategoryId::Unknown;
				break;
			case av::media::SourceCategory::Movie:
				return CategoryId::Movie;
				break;
			case av::media::SourceCategory::Discover:
				return CategoryId::Discover;
				break;
			case av::media::SourceCategory::Sport:
				return CategoryId::Sport;
				break;
			case av::media::SourceCategory::TVSeries:
				return CategoryId::TVSeries;
				break;
			case av::media::SourceCategory::Variety:
				return CategoryId::TVSeries;
				break;
			default:
				break;
		}
		return CategoryId::Unknown;
	}
}

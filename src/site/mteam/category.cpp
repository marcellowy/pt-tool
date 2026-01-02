#include "category.h"

namespace mteam {

	Category::Category(const av::media::SourceCategory category):m_category(category){
	
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
		case av::media::SourceCategory::Movie:
			return CategoryId::Movie;
		case av::media::SourceCategory::Discover:
			return CategoryId::Discover;
		case av::media::SourceCategory::Sport:
			return CategoryId::Sport;
		case av::media::SourceCategory::TVSeries:
			return CategoryId::TVSeries;
		case av::media::SourceCategory::Variety:
			return CategoryId::TVSeries;
		}
		return CategoryId::Unknown;
	}
}

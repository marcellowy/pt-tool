#include "category.h"

std::tstring mteam::category::to_string(Id id) {
	switch (id) {
	case Id::TVSeries: return TEXT("402");
	case Id::Discover:  return TEXT("404");
	case Id::Sport:  return TEXT("407");
	case Id::Movie: return TEXT("419");
	default:
		return TEXT("Unknown");
	}
	return TEXT("Unknown");
}

mteam::category::Id mteam::category::from_string(const std::tstring& str) {
	if (str == TEXT("402")) return mteam::category::Id::TVSeries;
	if (str == TEXT("404"))  return mteam::category::Id::Discover;
	if (str == TEXT("407"))  return mteam::category::Id::Sport;
	if (str == TEXT("419")) return mteam::category::Id::Movie;
	return mteam::category::Id::Unknown;
}
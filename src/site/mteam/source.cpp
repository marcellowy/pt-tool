#include "source.h"

#include <map>

#include "av_string.h"

namespace mteam {

	static std::map<SourceId, std::tstring> sourceIdMap = {
		{SourceId::Unknown, "Unknown"},
		{SourceId::_Bluray, "Unknown"},
		{SourceId::_CD, "Unknown"},
		{SourceId::_DVD, "Unknown"},
		{SourceId::_Encode, "Unknown"},
		{SourceId::_HDTV, "Unknown"},
		{SourceId::_Other, "Unknown"},
		{SourceId::_Remux, "Unknown"},
		{SourceId::_TV, "Unknown"},
		{SourceId::_Web_DL, "Unknown"},
	};

	Source::Source(const av::media::SourceId& source_id):m_source_id(source_id) {
	}

	SourceId Source::getid() {
		switch (m_source_id) {
		case av::media::SourceId::_Bluray:
			return SourceId::_Bluray;
			break;
		case av::media::SourceId::_CD:
			return SourceId::_CD;
			break;
		case av::media::SourceId::_DVD:
			return SourceId::_DVD;
			break;
		case av::media::SourceId::_Encode:
			return SourceId::_Encode;
			break;
		case av::media::SourceId::_HDTV:
			return SourceId::_HDTV;
			break;
		case av::media::SourceId::_TV:
			return SourceId::_TV;
			break;
		case av::media::SourceId::_Other:
			return SourceId::_Other;
			break;
		case av::media::SourceId::_Remux:
			return SourceId::_Remux;
			break;
		case av::media::SourceId::_WebDL:
			return SourceId::_WebDL;
			break;
		default:
			break;
		}
		return SourceId::Unknown;
	}

	std::tstring Source::getText() {
		auto id = getid();
		return sourceIdMap[id];
	}
}

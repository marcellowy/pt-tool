#include "source.h"

#include <map>

#include "av_string.h"

namespace mteam {

	static std::map<SourceId, std::tstring> sourceIdMapString = {
		{SourceId::Unknown, TEXT("Unknown")},
		{SourceId::_Bluray, TEXT("Unknown")},
		{SourceId::_CD, TEXT("Unknown")},
		{SourceId::_DVD, TEXT("Unknown")},
		{SourceId::_Encode, TEXT("Unknown")},
		{SourceId::_HDTV, TEXT("HDTV")}, // 我发的都是HDTV,所以只知道hdtv
		{SourceId::_Other, TEXT("Unknown")},
		{SourceId::_Remux, TEXT("Unknown")},
		{SourceId::_TV, TEXT("Unknown")},
		{SourceId::_WebDL, TEXT("Unknown")},
	};

	Source::Source(const av::media::SourceId& source_id):m_source_id(source_id) {
	}

	void Source::setExternalSourceId(const av::media::SourceId& source_id) {
		m_source_id = source_id;
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
		return sourceIdMapString[id];
	}
}

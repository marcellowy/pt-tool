#ifndef MTEAM_SOURCE_H_
#define MTEAM_SOURCE_H_


#include <map>

#include "av_string.h"

#include "av_media_info.h"

namespace mteam {
	enum class SourceId {
		Unknown = 0,
		_WebDL = 8,
		_Bluray = 1,
		_Remux  = 4,
		_Encode = 9,
		_HDTV   = 5,
		_TV		= 5,
		_DVD	= 3,
		_CD		= 7,
		_Other  = 6
	};

	class Source
	{
	public:
		Source() = default;
		Source(const av::media::SourceId& source_id);
		~Source() = default;
		SourceId getid();
		std::tstring getText();
	private:
		av::media::SourceId m_source_id;
	};

}


#endif

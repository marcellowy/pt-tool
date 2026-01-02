#ifndef MTEAM_MTEAM_H_
#define MTEAM_MTEAM_H_

#include "av_media_info.h"
#include "category.h"

namespace mteam {

class Mteam
{
public:
	Mteam() = default;
	Mteam(const av::media::Source& source);
	~Mteam();
	
private:
	av::media::Source m_source;
	Category m_category;
};

}
#endif

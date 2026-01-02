#ifndef SITE_H_
#define SITE_H_

#include "av_media_info.h"

class Site {
public:
	virtual bool publish(const av::media::Source& source) = 0;
	virtual ~Site() = default;
};

#endif // !SITE_H_

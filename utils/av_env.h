#ifndef AV_ENV_H_
#define AV_ENV_H_

#include <cstdlib>

#include "av_string.h"

namespace av {
	namespace env {
		bool is_dev();
		std::tstring get(const std::tstring& key);		
	}
}

#endif // !AV_ENV_H_

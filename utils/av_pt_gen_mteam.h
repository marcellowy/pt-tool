#ifndef AV_PT_GEN_MTEAM_H
#define AV_PT_GEN_MTEAM_H

#include "nlohmann/json.hpp"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_string.h"
#include "av_http.h"
#include "av_log.h"
#include  "av_pt_gen.h"

namespace av {
	namespace ptgen {
		bool getByMteam(const std::tstring& douban_url, const std::tstring& api_key, Douban& douban);
	}
}



#endif //AV_PT_GEN_MTEAM_H

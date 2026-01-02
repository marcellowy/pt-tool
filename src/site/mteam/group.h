#ifndef MTEAM_GROUP_H_
#define MTEAM_GROUP_H_

#include <map>

#include "av_string.h"

namespace mteam {
    namespace group { // 官组
        enum class Id {
            Unknown = 0,
            TPTV = 43,
            MWeb = 44,
            MTeam = 9
        };
        std::map<int64_t, std::tstring> enum_group();
        std::string getText(int64_t id);
    }
};

#endif

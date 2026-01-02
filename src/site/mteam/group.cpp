#include "group.h"
namespace mteam {
    namespace group { // 官组

        static std::map<int64_t, std::tstring> groupMapString = {
                {static_cast<int64_t>(Id::Unknown), TEXT("Unknown")},
                { static_cast<int64_t>(Id::TPTV), TEXT("TPTV") },
                { static_cast<int64_t>(Id::MWeb), TEXT("MWeb") },
                { static_cast<int64_t>(Id::MTeam), TEXT("MTeam") },
        };

        std::map<int64_t, std::tstring> enum_group() {
            return groupMapString;
        }

        std::tstring getText(int64_t id) {
            auto it = groupMapString.find(id);
            if (it != groupMapString.end()) {
                return it->second;
            }
            return groupMapString[static_cast<int64_t>(Id::Unknown)];
        }


    }
}

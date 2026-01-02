#include "group.h"
namespace mteam {
    namespace group { // 官组

        std::map<int64_t, std::tstring> enum_group() {
            return std::map<int64_t, std::tstring>{
                {static_cast<int64_t>(Id::Unknown), TEXT("Unknown")},
                { static_cast<int64_t>(Id::TPTV), TEXT("TPTV") },
                { static_cast<int64_t>(Id::MWeb), TEXT("MWeb") },
                { static_cast<int64_t>(Id::MTeam), TEXT("MTeam") },
            };
        }

        std::string getText(int64_t id) {
            Id v = static_cast<Id>(id);
            auto m = enum_group();
            if (m.find(v) == m.end()) {
                return m[Id::Unknown];
            }
            return m[v];
        }


    }
}

#ifndef AV_PT_GEN_H_
#define AV_PT_GEN_H_

#include "nlohmann/json.hpp"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_string.h"
#include "av_http.h"
#include "av_log.h"

using json = nlohmann::json;

namespace av {
	namespace ptgen {

		struct Director {
			std::string name;
		};

		struct Cast {
			std::string name;
		};

		struct Data {
			int64_t code;
			bool success;
			std::string message;
			std::string version;
			std::string chinese_title;
			std::string year;
			std::vector<std::string> region;
			std::vector<std::string> genre;
			std::vector<std::string> trans_title;
			std::vector<std::string> this_title;
			std::string format;
			std::vector<Director> director;
			std::vector<Cast> cast;
			std::string imdb_link;
			std::string imdb_id;
			std::string poster;
		};

		struct Douban {
			std::string name_chs;
			std::string sub_title;
			std::string year;
			std::string name_eng;
			std::string imdb_link;
			std::string poster_img;
			std::string description;
		};
		
		bool get(const std::tstring& url, Douban& douban);
	}
}

#endif

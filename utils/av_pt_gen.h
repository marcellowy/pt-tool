#ifndef AV_PT_GEN_H_
#define AV_PT_GEN_H_

#include "nlohmann/json.hpp"

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

		void from_json(const nlohmann::json& j, Director& item) {
			j.at("name").get_to(item.name);
		}

		void from_json(const nlohmann::json& j, Cast& item) {
			j.at("name").get_to(item.name);
		}

		void from_json(const nlohmann::json& j, Data& data) {
			data.code = j.value("ret", -1);
			data.success = j.value("success", false);
			data.message = j.value("msg", "");
			data.version = j.value("version", "");
			data.chinese_title = j.value("chinese_title", "");
			data.year = j.value("year", "");
			if (j.contains("region") && j["region"].is_array()) {
				data.region = j["region"].get<std::vector<std::string>>();
			}
			if (j.contains("genre") && j["genre"].is_array()) {
				data.genre = j["genre"].get<std::vector<std::string>>();
			}
			if (j.contains("trans_title") && j["trans_title"].is_array()) {
				data.trans_title = j["trans_title"].get<std::vector<std::string>>();
			}
			if (j.contains("this_title") && j["this_title"].is_array()) {
				data.this_title = j["this_title"].get<std::vector<std::string>>();
			}
			data.format = j.value("format", "");
			if (j.contains("director") && j["director"].is_array()) {
				j.at("director").get_to(data.director);
			}
			if (j.contains("cast") && j["cast"].is_array()) {
				j.at("cast").get_to(data.cast);
			}
			data.imdb_link = j.value("imdb_link", "");
			data.imdb_id = j.value("imdb_id", "");
			data.poster = j.value("poster", "");
		}

		bool get(const std::tstring& url, Data& data) {
			std::string a;
			av::http::Client client;
			av::http::Response resp;
			if (!client.get(av::str::toT(url), resp)) {
				loge("http failed");
				return false;
			}
			if (!resp.isOk()) {
				loge("http failed");
				return false;
			}

			try {
				json obj;
				auto j = obj.parse(av::str::toA(resp.body));
				data = j.get<Data>();
				if (data.success) {
					return true;
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				loge("parse error {}, {}", e.what(), av::str::toA(resp.body));
				return false;
			}
			catch (const std::exception& e) {
				loge("parse error {}, {}", e.what(), av::str::toA(resp.body));
			}
			return false;

		}
	}
}

#endif

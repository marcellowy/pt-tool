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

		struct Douban {
			std::string name_chs;
			std::string sub_title;
			std::string year;
			std::string name_eng;
			std::string imdb_link;
			std::string poster_img;
			std::string description;
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
			data.chinese_title = av::str::trim(j.value("chinese_title", ""));
			data.year = av::str::trim(j.value("year", ""));
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
			data.imdb_link = av::str::trim(j.value("imdb_link", ""));
			data.imdb_id = av::str::trim(j.value("imdb_id", ""));
			data.poster = av::str::trim(j.value("poster", ""));
		}

		bool get(const std::tstring& url, Douban& douban) {
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
			Data d;
			try {
				json obj;
				auto j = obj.parse(av::str::toA(resp.body));
				d = j.get<Data>();
				if (!d.success) {
					loge("parse failed; {}", av::str::toA(resp.body));
					return false;
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				loge("parse error {}, {}", e.what(), av::str::toA(resp.body));
				return false;
			}
			catch (const std::exception& e) {
				loge("parse error {}, {}", e.what(), av::str::toA(resp.body));
			}

#ifdef DEBUG_PT_GEN
			logi("version {}", d.version);
			logi("chinese title {}", d.chinese_title);
			//logi("format {}", d.format);
			logi("code {}", d.code);
			logi("success {}", d.success);
			logi("message {}", d.message);
			logi("year {}", d.year);
			logi("imdb_id {}", d.imdb_id);
			logi("imdb_link {}", d.imdb_link);
			logi("poster {}", d.poster);
			//logi("{}", fmt::join(d.director, ","));
			//logi("{}", fmt::join(d.cast, ","));
			std::vector<std::string> directors;
			for (auto tmp : d.director) {
				directors.push_back(tmp.name);
			}
			std::vector<std::string> casts;
			for (auto tmp : d.cast) {
				casts.push_back(tmp.name);
			}
			logi("directors: {}", fmt::join(directors, ","));
			logi("casts: {}", fmt::join(casts, ","));
			logi("region: {}", fmt::join(d.region, ","));
			logi("genre: {}", fmt::join(d.genre, ","));
			logi("trans_title: {}", fmt::join(d.trans_title, ","));
			logi("this_title: {}", fmt::join(d.this_title, ","));
#endif // DEBUG

			if (!d.chinese_title.empty()) {
				douban.sub_title += d.chinese_title;
				douban.name_chs = d.chinese_title;
			}

			if(!d.year.empty()){
				douban.sub_title += " | " + d.year;
				douban.year = d.year;
			}

			if (d.region.size() > 0) {
				std::string tmp = av::str::trim(d.region[0]);
				douban.sub_title += " | " + tmp;
			}

			if (d.genre.size() > 0) {
				auto tmp = fmt::format("{}", fmt::join(d.genre, " "));
				douban.sub_title += " | " + tmp;
			}

			auto filter_name = [](const std::string& name) {
				size_t pos = name.find_first_of(" ");
				if (pos == std::string::npos) {
					// no space
					return name;
				}
				auto tmp = name.substr(0, pos);
				tmp = av::str::trim(tmp);
				if (tmp != "") {
					return tmp;
				}
			};

			if (d.director.size() > 0) {
				std::string cn_name = "";
				size_t count = 0;
				for (auto& direc : d.director) {
					cn_name += " " + filter_name(direc.name);
					if (count > 5) {
						break;
					}
					count++;
				}
				if(!cn_name.empty())
					douban.sub_title += " |" + cn_name;
			}

			if (d.cast.size() > 0) {
				std::string cn_name = "";
				size_t count = 0;
				for (auto& cas : d.cast) {
					cn_name += " " + filter_name(cas.name);
					if (count > 8) {
						break;
					}
					count++;
				}
				if(!cn_name.empty())
					douban.sub_title += " |" + cn_name;
			}

			// chekc chinese
			auto has_chinese_utf8 = [](const std::string& str) {
				return std::any_of(str.begin(), str.end(), [](unsigned char c) {
					return c > 127;
				});
			};

			// get english name from trans title vector
			if (!d.trans_title.empty()) {
				for (auto& tmp : d.trans_title) {
					tmp = av::str::trim(tmp);
					if (!has_chinese_utf8(tmp)) {
						douban.name_eng = tmp;
						break;
					}
				}
			}

			// if trans title no english name and this title not empty
			if (douban.name_eng.empty() && !d.this_title.empty()) {
				for (auto& tmp : d.this_title) {
					tmp = av::str::trim(tmp);
					if (!has_chinese_utf8(tmp)) {
						douban.name_eng = tmp;
						break;
					}
				}
			}

			// imdb
			auto imdb_link = av::str::trim(d.imdb_link);
			if (!imdb_link.empty()) {
				douban.imdb_link = imdb_link;
			}

			// poster img
			if (!d.poster.empty()) {
				auto tmp = av::str::toT(d.poster);
				av::str::replace_all(tmp, TEXT("//img1."), TEXT("//img2."));
				douban.poster_img = av::str::toA(tmp);
			}

			// description
			std::tstring f_ = av::str::toT(d.format);
			av::str::replace(f_,
				TEXT("https://img1.doubanio.com/view/photo"),
				TEXT("https://img2.doubanio.com/view/photo"));
			douban.description = av::str::toA(f_);
			
			return true;
		}
	}
}

#endif

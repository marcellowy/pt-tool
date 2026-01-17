//
// Created by Marcello on 2026/1/17.
//

#include "av_pt_gen_mteam.h"
#include  "av_http_v2.h"
namespace av {
	namespace ptgen {

		bool getByMteam(const std::tstring& douban_url, const std::tstring& api_key, Douban& douban) {

			av::http_v2::Client cli;
			av::http_v2::Form form;
			form.kv["code"] = av::str::toA(douban_url);
			av::http_v2::Header header;
			header.kv["x-api-key"] = av::str::toA(api_key);

			auto result = cli.post("https://api.m-team.cc/api/media/douban/infoV2", header, form);
			if (!result) {
				logw("post failed");
				return false;
			}
			if (result->status != 200) {
				logw("post failed, status {}", result->status);
				return false;
			}
			logi("{}", result->body);
			Data d;
			try {
				json obj;
				auto j = obj.parse(result->body);
				if (!j.contains("code") || j["code"].is_number_integer()) {
					logw("no code");
					return false;
				}
				d.code = std::atoi(j["code"].get<std::string>().c_str());
				if (d.code != 0) {
					return false;
				}
				d.message = j["message"].get<std::string>();
				if (!j.contains("data")) {
					logw("no data");
					return false;
				}
				json data = j["data"];
				if (data.is_null()) {
					// no data
					logw("data is null, no douban info");
					return false;
				}

				if (data.contains("title")) {
					d.chinese_title = data["title"].get<std::string>();
				}
				if (data.contains("year")) {
					d.year = data["year"].get<std::string>();
				}
				if (data.contains("countries")) {
					d.region = data["countries"].get<std::vector<std::string>>();
				}
				if (data.contains("genres")) {
					d.genre = data["genres"].get<std::vector<std::string>>();
				}
				if (data.contains("aka")) {
					d.trans_title = data["aka"].get<std::vector<std::string>>();
				}
				if (data.contains("format")) {
					d.format = data["format"].get<std::string>();
				}
				if (data.contains("directors")) {
					std::vector<json> directors = data["directors"].get<std::vector<json>>();
					for (const auto& director : directors) {
						Director d_tmp;
						d_tmp.name = director.contains("name") ? director["name"].get<std::string>() : "";
						d.director.push_back(d_tmp);
					}
				}
				if (data.contains("actors")) {
					std::vector<json> actors = data["actors"].get<std::vector<json>>();
					for (const auto& actor : actors) {
						Cast d_tmp;
						d_tmp.name = actor.contains("name") ? actor["name"].get<std::string>() : "";
						d.cast.push_back(d_tmp);
					}
				}
				if (data.contains("imdbId")) {
					d.imdb_id = data["imdbId"].get<std::string>();
					d.imdb_link = fmt::format("https://www.imdb.com/title/{}/", d.imdb_id);
				}
				if (data.contains("coverUrl")) {
					d.poster = data["coverUrl"].get<std::string>();
				}
			}
			catch (const nlohmann::json::parse_error& e) {
				loge("parse error {}, {}", e.what(), av::str::toA(result->body));
				return false;
			}
			catch (const std::exception& e) {
				loge("parse error {}, {}", e.what(), av::str::toA(result->body));
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

			if (!d.year.empty()) {
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

			auto filter_name = [](const std::string& name) -> std::string {
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
				return name;
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
				if (!cn_name.empty())
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
				if (!cn_name.empty())
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
			logi("english name: {}", douban.name_eng);

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
			douban.poster_img = d.poster;
			douban.description = d.format;

			// poster img
			// if (!d.poster.empty()) {
			// 	auto tmp = av::str::toT(d.poster);
			// 	av::str::replace_all(tmp, TEXT("//img1."), TEXT("//img2."));
			// 	douban.poster_img = av::str::toA(tmp);
			// }

			// description
			// std::tstring f_ = av::str::toT(d.format);
			// av::str::replace(f_,
			// 	TEXT("https://img1.doubanio.com/view/photo"),
			// 	TEXT("https://img2.doubanio.com/view/photo"));
			//douban.description = av::str::toA(f_);
			return true;
		}
	}
}
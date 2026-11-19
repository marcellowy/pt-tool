#include "server_on_rename.h"
#include <nlohmann/json.hpp>
#include "av_log.h"
#include <filesystem>
#include <fmt/core.h>

namespace fs = std::filesystem;

class RenameReq {
public:
	std::string filename;
	std::string path;
	std::string source_id;
	std::string category_id;
	std::string name_chs;
	std::string name_eng;
	std::string sub_title;
	std::string name_prefix;
	std::string dou_ban_id;
	std::string season;
	std::string year;
	bool is_complete;

	// JSON serialization
	friend void to_json(nlohmann::json& j, const RenameReq& p) {
		j = nlohmann::json{
			{"filename", p.filename},
			{"path", p.path},
			{"source_id", p.source_id},
			{"category_id", p.category_id},
			{"name_chs", p.name_chs},
			{"name_eng", p.name_eng},
			{"sub_title", p.sub_title},
			{"name_prefix", p.name_prefix},
			{"dou_ban_id", p.dou_ban_id},
			{"season", p.season},
			{"year", p.year},
			{"is_complete", p.is_complete}
		};
	}
	friend void from_json(const nlohmann::json& j, RenameReq& p) {
		if (j.contains("filename") && j["filename"].is_string()) {
			p.filename = j["filename"].get<std::string>();
		}
		if (j.contains("path") && j["path"].is_string()) {
			p.path = j["path"].get<std::string>();
		}
		if (j.contains("source_id") && j["source_id"].is_string()) {
			p.source_id = j["source_id"].get<std::string>();
		}
		if (j.contains("category_id") && j["category_id"].is_string()) {
			p.category_id = j["category_id"].get<std::string>();
		}
		if (j.contains("name_chs") && j["name_chs"].is_string()) {
			p.name_chs = j["name_chs"].get<std::string>();
		}
		if (j.contains("name_eng") && j["name_eng"].is_string()) {
			p.name_eng = j["name_eng"].get<std::string>();
		}
		if (j.contains("sub_title") && j["sub_title"].is_string()) {
			p.sub_title = j["sub_title"].get<std::string>();
		}
		if (j.contains("name_prefix") && j["name_prefix"].is_string()) {
			p.name_prefix = j["name_prefix"].get<std::string>();
		}
		if (j.contains("dou_ban_id") && j["dou_ban_id"].is_string()) {
			p.dou_ban_id = j["dou_ban_id"].get<std::string>();
		}
		if (j.contains("season") && j["season"].is_string()) {
			p.season = j["season"].get<std::string>();
		}
		if (j.contains("year") && j["year"].is_string()) {
			p.year = j["year"].get<std::string>();
		}
		if (j.contains("is_complete") && j["is_complete"].is_boolean()) {
			p.is_complete = j["is_complete"].get<bool>();
		}
	};
};

void Server::onRename(const httplib::Request& req, httplib::Response& res) {
	RenameReq req_obj;
	try {
		auto obj = nlohmann::json::parse(req.body);
		obj.get_to(req_obj);
	}
	catch (const std::exception& e) {
		logw("exception {}", e.what());
		res.status = 400;
		return;
	};

	//
	if (req_obj.is_complete) {
		return;
	}

	//
	std::error_code ec;
	fs::rename(req_obj.path, fs::path(req_obj.path).parent_path() / fmt::format("0000@[{}][{}][{}][{}][{}][{}][{}][{}].ts",
		req_obj.category_id,
		req_obj.name_chs,
		req_obj.name_prefix,
		req_obj.sub_title,
		req_obj.year,
		req_obj.dou_ban_id,
		req_obj.season,
		req_obj.name_eng), ec);
	if (ec) {
		logw("rename failed: {}, {}, {}", 
			ec.message(), ec.value(), ec.category().name());
	}

	//
	Response<std::string> resp;
	resp.code = 0;
	resp.message = "success";
	resp.data = "";
	const nlohmann::json j = resp;
	res.set_content(j.dump(), "application/json");
};

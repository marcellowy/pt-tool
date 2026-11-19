#include "server_on_read_file.h"
#include <vector>
#include <nlohmann/json.hpp>
#include "publish.h"
#include "config.h"

class File {
public:
	bool is_complete;
	std::string filename;
	std::string path;

	//	JSON serialization
	friend void to_json(nlohmann::json& j, const File& p) {
		j = nlohmann::json{
			{"is_complete", p.is_complete},
			{"filename", p.filename},
			{"path", p.path}
		};
	}
};

class ReadFileRes {
public:
	std::vector<File> files;

	//	JSON serialization
	friend void to_json(nlohmann::json& j, const ReadFileRes& p) {
		j = nlohmann::json{
			{"files", p.files}
		};
	}
};

void Server::onReadFile(const httplib::Request& req, httplib::Response& res) {
	auto& config = Config::instance();
	PublishError error = PublishError::Success;

	//
	ReadFileRes res_obj;

	//
	Publish publish(config.mteam.seed_dir);
	auto arr = publish.readDir(error);
	for (auto& tmp : arr)
	{
		if (!publish.getSiteType(tmp)) {
			// 解析站点类型失败
			File file;
			file.is_complete = false;
			file.filename = av::str::toA(tmp.name);
			file.path = av::str::toA(tmp.fullpath);
			res_obj.files.push_back(std::move(file));
		}
	}
	nlohmann::json j = res_obj;
	res.set_content(j.dump(), "application/json");
}

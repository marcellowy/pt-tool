#include "config.h"
#include <iostream>

bool Config::parse(const std::tstring& toml_file) {
	std::tstring tv_name_file = TEXT("");
	try {

		// config.toml
		const auto config = toml::parse_file(av::str::toA(toml_file));
		log.dir = av::str::toT(config["log"]["dir"].value_or(log.dir));
		log.max_file_size = config["log"]["max_size"].value_or(log.max_file_size);
		log.max_file_count = config["log"]["max_count"].value_or(log.max_file_count);
		auto log_level = config["log"]["level"].value_or("info");
		log.level = spdlog::level::from_str(log_level);

		// server
		server.host = av::str::toT(config["server"]["host"].value_or(""));
		server.port = config["server"]["port"].value_or(0);

		// rapidapi
		rapidapi.key = av::str::toT(config["rapidapi"]["key"].value_or(""));
		rapidapi.host = av::str::toT(config["rapidapi"]["host"].value_or(""));

		// tv map file
		tv_name_file = av::str::toT(config["tvname"]["file"].value_or(""));
		if (tv_name_file.empty()) {
			loge("tv_name_file is empty");
			return false;
		}

		// tv name,  config_tv_name.toml
		const auto config_tv = toml::parse_file(av::str::toA(tv_name_file));
		if (!config_tv["TVNameMap"].is_array()) {
			loge("TVNameMap not array");
			return false;
		}
		if (auto arr = config_tv["TVNameMap"].as_array()) {
			for (const auto& item : *arr) {
				const auto& table = *item.as_table();
				TVName n;
				n.match = av::str::toT(table["match"].value_or(""));
				n.title_prefix = av::str::toT(table["titlePrefix"].value_or(""));
				n.sub_title_prefix = av::str::toT(table["subTitleText"].value_or(""));
				tv_name.emplace_back(n);
			}
		}

		// mteam
		mteam.api_key = av::str::toT(config["mteam"]["api_key"].value_or(""));
		mteam.api_url = av::str::toT(config["mteam"]["api_url"].value_or(""));
		mteam.img_api_key = av::str::toT(config["mteam"]["img_api_key"].value_or(""));
		mteam.img_api_url = av::str::toT(config["mteam"]["img_api_url"].value_or(""));

		// tgbot
		tgbot.token = av::str::toT(config["tgbot"]["token"].value_or(""));
		tgbot.chat_id = av::str::toT(config["tgbot"]["chat_id"].value_or(""));
		

	}
	catch (const toml::parse_error& e) {
		loge("parse {} {} failed, err {}", 
			av::str::toA(toml_file), av::str::toA(tv_name_file), e.what());
		return false;
	}

	return true;
}

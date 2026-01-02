#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_

#include <string>
#include <vector>

#include "toml++/toml.h"

#include "av_log.h"
#include "av_singleton.h"
#include "av_string.h"
#include "spdlog/spdlog.h"

struct TableLog {
	std::tstring dir = TEXT("logs");	// log to dir
	size_t max_file_size = 5 * 1024 * 1024;	// file max size
	size_t max_file_count = 3;				// file max count
	spdlog::level::level_enum level;	// debug
};

struct TableServer {
	std::tstring host = TEXT("0.0.0.0");	// server host
	uint16_t port = 8000;			// server port
};

struct TVName {
	std::tstring match = TEXT("");
	std::tstring title_prefix = TEXT("");
	std::tstring sub_title_prefix = TEXT("");
};

struct Rapidapi {
	std::tstring key = TEXT("");
	std::tstring host = TEXT("");
};

struct Mteam {
	int64_t group_id = 0;
	int64_t source_id = 0;
	std::tstring api_key = TEXT("");
	std::tstring api_url = TEXT("");
	std::tstring img_api_key = TEXT("");
	std::tstring img_api_url = TEXT("");
};

struct TGBot {
	std::tstring token = TEXT("");
	std::tstring chat_id = TEXT("");
};

struct PTGen {
	std::tstring url = TEXT("");
};

class Config: public av::Singleton<Config> {
	friend class Singleton<Config>;
public:
	TableLog log;
	TableServer server;
	std::vector<TVName> tv_name = {};
	Rapidapi rapidapi;
	Mteam mteam;
	TGBot tgbot;
	PTGen ptgen;
public:
	bool parse(const std::tstring& toml_file);
};

#endif

#ifndef TEST_CONFIG_H_
#define TEST_CONFIG_H_

#include <string>
#include <vector>

#include "toml++/toml.h"

#include "av_log.h"
#include "av_singleton.h"
#include "av_string.h"

struct TableLog {
	std::tstring dir = TEXT("");				// log to dir
	size_t max_size = 5 * 1024 * 1024;	// file max size
	size_t max_count = 3;				// file max count
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

class Config: public av::Singleton<Config> {
	friend class Singleton<Config>;
public:
	TableLog log;
	TableServer server;
	std::vector<TVName> tv_name = {};
	Rapidapi rapidapi;
public:
	bool parse(const std::tstring& toml_file);
};

#endif
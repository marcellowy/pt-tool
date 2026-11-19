#ifndef PT_TOOL_SERVER_H_
#define PT_TOOL_SERVER_H_

#include <httplib.h>
#include <nlohmann/json.hpp>

template<typename T>
class Response {
public:
	int64_t code;
	std::string message;
	T data;

	friend void to_json(nlohmann::json& j, const Response<T>& p) {
		j = nlohmann::json{
			{"code", p.code},
			{"message", p.message},
			{"data", p.data}
		};
	}
};


class Server {
public:
	Server();
	~Server();
	void Start(const std::string& host, int port);
private:
	void router();

	void onRename(const httplib::Request& req, httplib::Response& res);
	void onReadFile(const httplib::Request& req, httplib::Response& res);
private:
	httplib::Server server_;
};

#endif

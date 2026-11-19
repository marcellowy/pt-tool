#ifndef PT_TOOL_SERVER_H_
#define PT_TOOL_SERVER_H_

#include <httplib.h>

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

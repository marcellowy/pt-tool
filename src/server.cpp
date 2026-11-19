#include "server.h"

#include "av_log.h"

Server::Server() {
}

Server::~Server() {
}

void Server::router() {

	// 读取文件
	server_.Post("/api/v1/at/read-file", [this](const httplib::Request& req, httplib::Response& res) {
		this->onReadFile(req, res);
		});

	// 重命名文件
	server_.Post("/api/v1/at/publish", [this](const httplib::Request& req, httplib::Response& res) {
		this->onRename(req, res);
		});

	// 2. 挂载静态文件目录（例如当前目录下的 ./html 文件夹）
	// 访问 http://localhost:8080/html/ 会自动查找 ./html/index.html
	auto ret = server_.set_mount_point("/html", "./html");
	if (!ret) {
		std::cerr << "静态目录 ./html 不存在！" << std::endl;
		return;
	}
}

void Server::Start(const std::string& host, int port) {
	if (!server_.bind_to_port(host.c_str(), port)) {
		logw("bind failed");
		return;
	}
	logi("server start on {}:{}", host, port);
	router();
	if (!server_.listen_after_bind()) {
		logw("listen failed");
	}
}

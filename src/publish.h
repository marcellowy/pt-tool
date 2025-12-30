#ifndef TEST_PUBLISH_H_
#define TEST_PUBLISH_H_

#include <string>
#include <vector>

#include "defined.h"


class Publish
{
public:
	Publish();
	Publish(const std::tstring& dir);
	~Publish();
	bool start();
	bool stop();
protected:
	std::vector<PublishObj> readDir();
	bool getSiteType(PublishObj& obj);
	
	bool processDir(const std::tstring& path);
	bool processFile(PublishObj& obj);

	// map tv name
	void tvname(PublishObj& obj);
private:
	std::tstring dir_;
};

#endif

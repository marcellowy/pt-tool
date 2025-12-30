#ifndef TEST_PUBLISH_H_
#define TEST_PUBLISH_H_

#include <string>
#include <vector>

#include "defined.h"
#include "av_media_info.h"

using namespace av::media;


class Publish
{
public:
	Publish();
	Publish(const std::tstring& dir);
	~Publish();
	bool start();
	bool stop();
protected:
	std::vector<Source> readDir();
	bool getSiteType(Source& obj);
	
	bool processDir(const std::tstring& path);
	bool processFile(Source& obj);

	// map tv name
	void tvname(Source& obj);
private:
	std::tstring dir_;
};

#endif

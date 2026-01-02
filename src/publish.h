#ifndef TEST_PUBLISH_H_
#define TEST_PUBLISH_H_

#include <string>
#include <vector>

#include "defined.h"
#include "av_media_info.h"
#include "site.h"

using namespace av::media;


class Publish
{
public:
	Publish();
	Publish(std::shared_ptr<Site>& site, const std::tstring& dir);
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
	std::tstring m_dir;
	std::shared_ptr<Site> m_site;
};



#endif

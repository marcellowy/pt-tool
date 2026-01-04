#ifndef TEST_PUBLISH_H_
#define TEST_PUBLISH_H_

#include <string>
#include <vector>
#include <memory>

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
	
	int processDir(Source& obj);
	bool processFile(Source& obj);

	// map tv name
	void tvname(Source& obj);

	// 
	void setResolution(int64_t width, int64_t height, const av::media::ScanType& scan_type, Source& obj);
private:
	std::tstring m_dir;
	std::shared_ptr<Site> m_site;
};



#endif

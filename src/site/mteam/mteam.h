#ifndef MTEAM_MTEAM_H_
#define MTEAM_MTEAM_H_

#include "av_media_info.h"
#include "category.h"
#include "site.h"

namespace mteam {

	class Mteam : public Site
	{
	public:
		Mteam();
		bool publish(const av::media::Source& source) override;
		~Mteam();
	
	private:
		av::media::Source m_source;
		std::shared_ptr<Category> m_category;
	};

}
#endif

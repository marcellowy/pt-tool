#include "mteam.h"

namespace mteam {

	Mteam::Mteam() {
		
	}

	bool Mteam::publish(const av::media::Source& source) {
		m_source = source;
		m_category = std::make_shared<Category>(m_source.category);
		return true;
	}

	Mteam::~Mteam()
	{

	}
}

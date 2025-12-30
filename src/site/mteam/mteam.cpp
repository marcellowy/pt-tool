#include "mteam.h"

namespace mteam {

	Mteam::Mteam(const av::media::Source& source) : m_source(source)
	{
		m_category = mteam::Category(m_source.category);
	}

	Mteam::~Mteam()
	{

	}
}
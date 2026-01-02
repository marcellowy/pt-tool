#include "mteam.h"
#include "av_path.h"

namespace mteam {

	Mteam::Mteam() {
		
	}

	bool Mteam::publish(const av::media::Source& source) {
		m_external_source = source;
		//m_category = std::make_shared<Category>(m_source.category);
		//Category category(source.category);
		//CategoryId category_id = category.getid();
		//if (source.type == av::media::SourceType::File) {}

		m_video_codec.setSourceVideoCodec(source.video_codec);
		m_video_resolution.setSourceResolution(source.video_resolution);
		m_audio_codec.setSourceCodec(source.audio_codec);
		m_cateogry.setSourceCategory(source.category);
		m_source.setExternalSourceId(source.source_id);

		return true;
	}

	bool Mteam::parseName() {
		/*video::Codec video_codec(m_source.video_codec);
		video::Resolution video_resolution(m_source.video_resolution);
		audio::Codec audio_codec(m_source.audio_codec);*/
		return true;
	}

	Mteam::~Mteam()
	{

	}
}

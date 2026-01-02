#include "video_resolution.h"



namespace mteam {
	namespace video {
		static std::map<ResolutionId, std::tstring> ResolutionIdMapString = {
			{ResolutionId::Unknown, TEXT("Other")},
			{ResolutionId::_480, TEXT("480p")},
			{ResolutionId::_720, TEXT("720p")},
			{ResolutionId::_1080i, TEXT("1080i")},
			{ResolutionId::_1080p, TEXT("1080p")},
			{ResolutionId::_2k, TEXT("1440p")},
			{ResolutionId::_4k, TEXT("2160p")},
			{ResolutionId::_8k, TEXT("4320p")},
		};

		Resolution::Resolution(const av::media::SourceVideoResolution& resolution)
			:m_resolution(resolution)
		{
		}

		Resolution::~Resolution()
		{
		}

		void Resolution::setSourceResolution(const av::media::SourceVideoResolution& resolution) {
			m_resolution = resolution;
		}

		std::tstring Resolution::getText() {
			auto id = getid();
			if (ResolutionIdMapString.find(id) != ResolutionIdMapString.end()) {
				return ResolutionIdMapString[id];
			}
			return ResolutionIdMapString[ResolutionId::Unknown];
		}

		ResolutionId Resolution::getid() {
			switch (m_resolution) {
			case av::media::SourceVideoResolution::_480:
				return ResolutionId::_480;
				break;
			case av::media::SourceVideoResolution::_720:
				return ResolutionId::_720;
				break;
			case av::media::SourceVideoResolution::_1080i:
				return ResolutionId::_1080i;
				break;
			case av::media::SourceVideoResolution::_1080p:
				return ResolutionId::_1080p;
				break;
			case av::media::SourceVideoResolution::_2k:
				return ResolutionId::_2k;
				break;
			case av::media::SourceVideoResolution::_4k:
				return ResolutionId::_4k;
				break;
			case av::media::SourceVideoResolution::_8k:
				return ResolutionId::_8k;
				break;
			}
			return ResolutionId::Unknown;
		}
	}
}

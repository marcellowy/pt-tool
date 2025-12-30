#include "video_resolution.h"



namespace mteam {

	static std::map<VideoResolutionId, std::tstring> VideoResolutionIdMapString = {
		{VideoResolutionId::Unknown, TEXT("Other")},
		{VideoResolutionId::_480, TEXT("480p")},
		{VideoResolutionId::_720, TEXT("720p")},
		{VideoResolutionId::_1080i, TEXT("1080i")},
		{VideoResolutionId::_1080p, TEXT("1080p")},
		{VideoResolutionId::_2k, TEXT("1440p")},
		{VideoResolutionId::_4k, TEXT("2160p")},
		{VideoResolutionId::_8k, TEXT("4320p")},
	};

	VideoResolution::VideoResolution(const av::media::SourceVideoResolution& resolution)
		:m_resolution(resolution)
	{
	}

	VideoResolution::~VideoResolution()
	{
	}

	std::tstring VideoResolution::getText() {
		auto id = getid();
		if (VideoResolutionIdMapString.find(id) != VideoResolutionIdMapString.end()) {
			return VideoResolutionIdMapString[id];
		}
		return VideoResolutionIdMapString[VideoResolutionId::Unknown];
	}

	VideoResolutionId VideoResolution::getid(){
		switch (m_resolution) {
		case av::media::SourceVideoResolution::_480:
			return mteam::VideoResolutionId::_480;
			break;
		case av::media::SourceVideoResolution::_720:
			return mteam::VideoResolutionId::_720;
			break;
		case av::media::SourceVideoResolution::_1080i:
			return mteam::VideoResolutionId::_1080i;
			break;
		case av::media::SourceVideoResolution::_1080p:
			return mteam::VideoResolutionId::_1080p;
			break;
		case av::media::SourceVideoResolution::_2k:
			return mteam::VideoResolutionId::_2k;
			break;
		case av::media::SourceVideoResolution::_4k:
			return mteam::VideoResolutionId::_4k;
			break;
		case av::media::SourceVideoResolution::_8k:
			return mteam::VideoResolutionId::_8k;
			break;
		}
		return mteam::VideoResolutionId::Unknown;
	}
}
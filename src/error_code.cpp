#include "error_code.h"

namespace {
	std::unordered_map<PublishError, std::string> public_ = {
		{PublishError::Success, "Success"},
		{PublishError::ErrNoMediaInfo, "No mediainfo file"},
		{PublishError::ErrVideoCodecNotSupport, "Video codec not support"},
		{PublishError::ErrAudioCodecNotSupport, "Audio codec not support"},
		{PublishError::ErrCategoryNotSupport, "Category not support"},
		{PublishError::ErrParseJson, "Parse JSON error"},
		{PublishError::ErrParseCreatedTime, "Parse created time error"},
		{PublishError::ErrNoTSFile, "No TS file"},
		{PublishError::ErrTranslate, "Translate error"},
		{PublishError::ErrMoveImage, "Move image error"},
		{PublishError::ErrGetSiteType, "Get site type(parse filename) error"},
		{PublishError::ErrGetDoubanInfo, "Get douban info error"},
		{PublishError::ErrCaptureFrame, "Capture frame(screenshots) error"},
		{PublishError::ErrDirNotExists, "Dir not exists"},
		{PublishError::ErrParam, "Param error"},
		{PublishError::ErrPublishFailed, "Publish failed"},
    };
}

std::string publishErrorToString(PublishError code) {
	auto it = public_.find(code);
	if (it != public_.end()) {
		return it->second;
	}
	return "Unknown";
}

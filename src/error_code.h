#ifndef TEST_ERROR_CODE_H_
#define TEST_ERROR_CODE_H_

#include <string>
#include <unordered_map>

enum class ErrorCode {
    Success = 0,
    ErrParseConfigFileFailed,       // 解析配置文件错误
    ErrOpenLogFailed,               // 打开日志失败
    ErrServerParam,                 // 
    ErrServerBind,                  //
    ErrServerListen,                //
    ErrTimeNotReached,              //
    ErrStartCronFailed,
};

enum class PublishError: int {
    Success = 0,
    ErrNoMediaInfo,
    ErrVideoCodecNotSupport,
    ErrAudioCodecNotSupport,
    ErrCategoryNotSupport,
    ErrParseJson,
    ErrNoTSFile,
    ErrTranslate,
    ErrMoveImage,
    ErrMoveFile,
    ErrGetSiteType,
    ErrGetDoubanInfo,
    ErrCaptureFrame,
    ErrDirNotExists,
    ErrParam,
    ErrPublishFailed,
    ErrParseCreatedTime,
    ErrContactName,
};

std::string publishErrorToString(PublishError code);


#endif

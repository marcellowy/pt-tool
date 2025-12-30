#ifndef TEST_CODE_H_
#define TEST_CODE_H_

enum ErrorCode {
    Success = 0,
    ErrParseConfigFileFailed,       // 解析配置文件错误
    ErrOpenLogFailed,               // 打开日志失败
    ErrServerParam,                 // 
    ErrServerBind,                  //
    ErrServerListen,                //
};


#endif
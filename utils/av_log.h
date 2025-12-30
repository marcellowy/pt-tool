#ifndef AV_LOG_H_
#define AV_LOG_H_

#include <memory>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> getDefaultLogger();

#define logt(...) SPDLOG_LOGGER_CALL(getDefaultLogger(), spdlog::level::trace, __VA_ARGS__)
#define logd(...) SPDLOG_LOGGER_CALL(getDefaultLogger(), spdlog::level::debug, __VA_ARGS__)
#define logi(...) SPDLOG_LOGGER_CALL(getDefaultLogger(), spdlog::level::info, __VA_ARGS__)
#define logw(...) SPDLOG_LOGGER_CALL(getDefaultLogger(), spdlog::level::warn, __VA_ARGS__)
#define loge(...) SPDLOG_LOGGER_CALL(getDefaultLogger(), spdlog::level::err, __VA_ARGS__)
#define logc(...) SPDLOG_LOGGER_CALL(getDefaultLogger(), spdlog::level::critical, __VA_ARGS__)

#endif
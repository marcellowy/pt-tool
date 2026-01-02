#ifndef AV_LOG_H_
#define AV_LOG_H_

#include <memory>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> getDefaultLogger();

#define logd(...) \
	do {\
		auto _logger = getDefaultLogger();\
		if(_logger) {\
			SPDLOG_LOGGER_CALL(_logger, spdlog::level::debug, __VA_ARGS__); \
		} \
	} while(0)

#define logi(...) \
	do {\
		auto _logger = getDefaultLogger();\
		if(_logger) {\
			SPDLOG_LOGGER_CALL(_logger, spdlog::level::info, __VA_ARGS__); \
		} \
	} while(0)

#define logw(...) \
	do {\
		auto _logger = getDefaultLogger();\
		if(_logger) {\
			SPDLOG_LOGGER_CALL(_logger, spdlog::level::warn, __VA_ARGS__); \
		} \
	} while(0)

#define loge(...) \
	do {\
		auto _logger = getDefaultLogger();\
		if(_logger) {\
			SPDLOG_LOGGER_CALL(_logger, spdlog::level::err, __VA_ARGS__); \
		} \
	} while(0)

#define logc(...) \
	do {\
		auto _logger = getDefaultLogger();\
		if(_logger) {\
			SPDLOG_LOGGER_CALL(_logger, spdlog::level::critical, __VA_ARGS__); \
		} \
	} while(0)

#endif
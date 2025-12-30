#include "logger.h"
#include "spdlog/spdlog.h"

std::shared_ptr<spdlog::logger> getDefaultLogger() {
    auto logger = Logger::instance().getLogger();
    return logger;
}
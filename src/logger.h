#ifndef TEST_LOGGER_H_
#define TEST_LOGGER_H_

#include <iostream>

#include "spdlog/spdlog.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "av_singleton.h"
#include "av_log.h"
#include "av_base.h"
#include "av_string.h"
#include "av_path.h"
#include "config.h"


class Logger : public av::Singleton<Logger> {
	friend class Singleton<Logger>;

public:
	std::shared_ptr<spdlog::logger> getLogger() {
		return m_logger;
	}
    bool open() {
        std::tstring exe_name = av::path::get_exe_name();
#ifdef _WIN32
        av::str::replace(exe_name, TEXT(".exe"), TEXT(""));
#endif // _WIN32

        auto& config = Config::instance();
        const std::string pattern = "[%Y-%m-%d %H:%M:%S.%e][%l][%t][%s:%# %!] %v";

        // console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(config.log.level);
        console_sink->set_pattern(av::str::toA(pattern));

        // file sink
        auto log_dir = config.log.dir;
        if (!log_dir.empty()) {
            if (!av::path::dir_exists(log_dir)) {
                if (!av::path::create_dir(log_dir)) {
                    std::cout << "create " << av::str::toA(log_dir) << " failed" << std::endl;
                    return false;
                }
            }
        }
        std::tstring log_path = av::path::append(log_dir, exe_name + TEXT(".log"));

        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(av::str::toA(log_path), config.log.max_file_size, config.log.max_file_count);
        rotating_sink->set_level(config.log.level);
        rotating_sink->set_pattern(av::str::toA(pattern));

        // create logger
        m_logger = std::make_shared<spdlog::logger>("multi_sink_logger", spdlog::sinks_init_list{ console_sink, rotating_sink });
        m_logger->set_level(config.log.level);
        m_logger->flush_on(config.log.level);

        return true;
    }

protected:
	std::shared_ptr<spdlog::logger> m_logger;
};


#endif
#include "pch.h"

#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "Files.h"

namespace khiin {

void Logger::Initialize(std::filesystem::path log_folder) {
    log_folder /= "khiin_debug_log.txt";
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_folder.string(), 1048576, 2);
    file_sink->set_pattern("[%l] %s(%#) : [%Y-%m-%d %H:%M:%S.%e] [%!] %v");
    file_sink->set_level(spdlog::level::trace);
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvc_sink->set_pattern("%s(%#) : [%l] [%H:%M:%S.%e] %v");
    msvc_sink->set_level(spdlog::level::debug);
    
    auto sinks = std::vector<spdlog::sink_ptr>{file_sink, msvc_sink};
    auto logger = std::make_shared<spdlog::logger>(KHIIN_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

void Logger::Uninitialize() {
    spdlog::shutdown();
}

} // namespace khiin

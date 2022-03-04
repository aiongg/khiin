#include "pch.h"

#include "Logger.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "Files.h"

namespace khiin {

void Logger::Initialize(std::filesystem::path log_folder) {
    log_folder /= "debug_log.txt";
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_folder.string());
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    auto sinks = std::vector<spdlog::sink_ptr>{file_sink, msvc_sink};
    auto logger = std::make_shared<spdlog::logger>(KHIIN_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%s(%#) : [%l] [%Y-%m-%d %H:%M:%S.%e] [thread %t] [%!] %v");
    logger->flush_on(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

void Logger::Uninitialize() {
    spdlog::shutdown();
}

} // namespace khiin

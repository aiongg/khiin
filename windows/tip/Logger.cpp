#include "pch.h"

#include "engine/utils/logger.h"

#include "Logger.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace khiin::win32 {
namespace {

const std::string kLogFilename = "khiin_debug_log.txt";
constexpr size_t kLogMaxSize = 1048576 * 5;
constexpr size_t kNumLogFiles = 2;

} // namespace

void Logger::Initialize(std::filesystem::path log_folder) {
    log_folder /= kLogFilename;
    auto file_sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_folder.string(), kLogMaxSize, kNumLogFiles);
    file_sink->set_pattern("[%l] %s(%#) : [%Y-%m-%d %H:%M:%S.%e] [%!] %v");
    file_sink->set_level(spdlog::level::trace);
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvc_sink->set_pattern("%s(%#) : [%l] [%H:%M:%S.%e] %v");
    msvc_sink->set_level(spdlog::level::debug);
    
    auto sinks = std::vector<spdlog::sink_ptr>{file_sink, msvc_sink};
    auto logger = khiin::logger::setup(sinks);
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    spdlog::set_default_logger(logger);
    logger->info("Log started: {}", log_folder.string());
}

void Logger::Uninitialize() {
    khiin::logger::shutdown();
}

} // namespace khiin

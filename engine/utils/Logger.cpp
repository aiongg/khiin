#include "Logger.h"

#include "log.h"

#include "spdlog/sinks/msvc_sink.h"

namespace khiin {
namespace {

bool g_loaded = false;

} // namespace

void Logger::Initialize() {
    if (g_loaded) {
        return;
    }

    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvc_sink->set_pattern("%s(%#) : [%l] [%H:%M:%S.%e] %v");
    msvc_sink->set_level(spdlog::level::debug);

    auto sinks = std::vector<spdlog::sink_ptr>{msvc_sink};
    auto logger = std::make_shared<spdlog::logger>(KHIIN_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

    logger->info("Logging started");
    g_loaded = true;
}

void Logger::Uninitialize() {
    spdlog::shutdown();
    g_loaded = false;
}

} // namespace khiin

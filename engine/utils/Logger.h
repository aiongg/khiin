#pragma once

#include "log.h"

#include <memory>
#include <vector>

#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace khiin::logger {

inline std::shared_ptr<spdlog::logger> setup(std::vector<spdlog::sink_ptr> sinks) {
    auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME);

    if (logger == nullptr) {
        if (!sinks.empty()) {
            logger = std::make_shared<spdlog::logger>(KHIIN_DEFAULT_LOGGER_NAME, std::begin(sinks), std::end(sinks));
            spdlog::register_logger(logger);
        } else {
            logger = spdlog::stdout_color_mt(KHIIN_DEFAULT_LOGGER_NAME);
        }
    }

    return logger;
}

inline void shutdown() {
    spdlog::shutdown();
}

template <typename... Args>
void trace(fmt::format_string<Args...> fmt, Args &&...args) {
    if (auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME); logger != nullptr) {
        logger->trace(fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void debug(fmt::format_string<Args...> fmt, Args &&...args) {
    if (auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME); logger != nullptr) {
        logger->debug(fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void info(fmt::format_string<Args...> fmt, Args &&...args) {
    if (auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME); logger != nullptr) {
        logger->info(fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void warn(fmt::format_string<Args...> fmt, Args &&...args) {
    if (auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME); logger != nullptr) {
        logger->warn(fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void error(fmt::format_string<Args...> fmt, Args &&...args) {
    if (auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME); logger != nullptr) {
        logger->error(fmt, std::forward<Args>(args)...);
    }
}

template <typename... Args>
void critical(fmt::format_string<Args...> fmt, Args &&...args) {
    if (auto logger = spdlog::get(KHIIN_DEFAULT_LOGGER_NAME); logger != nullptr) {
        logger->critical(fmt, std::forward<Args>(args)...);
    }
}

} // namespace khiin::logger

#pragma once

#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
#endif

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT

#include <spdlog/spdlog.h>

#include <spdlog/fmt/ostr.h>

#define KHIIN_DEFAULT_LOGGER_NAME "khiin_logger"
#define KHIIN_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define KHIIN_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define KHIIN_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define KHIIN_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define KHIIN_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define KHIIN_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

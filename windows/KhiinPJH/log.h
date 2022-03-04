#pragma once

#include <comdef.h>

#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
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

#ifdef _DEBUG

#define CHECK_HRESULT(hr)                        \
    if (FAILED(hr)) {                            \
        auto msg = std::wstring(_com_error(hr).ErrorMessage()); \
        KHIIN_ERROR(L"[hresult 0x{:x}] {}", (uint32_t)hr, msg); \
    }

#define CHECK_RETURN_HRESULT(hr)                                \
    if (FAILED(hr)) {                                           \
        auto msg = std::wstring(_com_error(hr).ErrorMessage()); \
        KHIIN_ERROR(L"[hresult 0x{:x}] {}", (uint32_t)hr, msg); \
        return hr;                                              \
    }

#else
#define D(...)
#define CHECK_RETURN_HRESULT(hr)
#endif

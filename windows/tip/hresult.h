#pragma once

#include "log.h"

#include <comdef.h>

#define CHECK_HRESULT(hr)                                       \
    if (FAILED(hr)) {                                           \
        auto msg = std::wstring(_com_error(hr).ErrorMessage()); \
        KHIIN_ERROR(L"[hresult 0x{:x}] {}", (uint32_t)hr, msg); \
    }

#define CHECK_RETURN_HRESULT(hr)                                \
    if (FAILED(hr)) {                                           \
        auto msg = std::wstring(_com_error(hr).ErrorMessage()); \
        KHIIN_ERROR(L"[hresult 0x{:x}] {}", (uint32_t)hr, msg); \
        return hr;                                              \
    }

#define TRY_FOR_HRESULT try {

#define CATCH_FOR_HRESULT                    \
    }                                        \
    catch (winrt::hresult_error const &ex) { \
        CHECK_RETURN_HRESULT(ex.code());     \
    }                                        \
    return S_OK;

// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for
// future builds. This also affects IntelliSense performance, including code
// completion and many code browsing features. However, files listed here are
// ALL re-compiled if any one of them is updated between builds. Do not add
// files here that you will be updating frequently as this negates the
// performance advantage.

#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// add headers that you want to pre-compile here

#include <functional>

// Windows Header Files
#include <Unknwn.h>

#include <ShlObj_core.h>
#include <ctffunc.h>
#include <d2d1_1.h>
#include <d2d1effects.h>
#include <dwmapi.h>
#include <dwrite_3.h>
#include <InputScope.h>
#include <msctf.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <wincodec.h>
#include <windows.h>
#include <windowsx.h>
#include <winrt/base.h>

#pragma comment(lib, "windowsapp")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "shcore")

#include "hresult.h"
#include "log.h"
#include "resource.h"

namespace winrt {

// We need this so that winrt accepts our interface for both (/all) base & derived classes
// https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/author-coclasses#implement-a-com-interface-that-derives-from-another
template <>
inline bool is_guid_of<ITfTextInputProcessorEx>(guid const &id) noexcept {
    return is_guid_of<ITfTextInputProcessorEx, ITfTextInputProcessor>(id);
}

template <>
inline bool is_guid_of<ITfCandidateListUIElementBehavior>(guid const &id) noexcept {
    return is_guid_of<ITfCandidateListUIElementBehavior, ITfCandidateListUIElement, ITfUIElement>(id);
}

} // namespace winrt

// Windows 11 polyfill

#define DWMWA_WINDOW_CORNER_PREFERENCE (33)

typedef enum {
    DWMWCP_DEFAULT = 0,
    DWMWCP_DONOTROUND = 1,
    DWMWCP_ROUND = 2,
    DWMWCP_ROUNDSMALL = 3,
} DWM_WINDOW_CORNER_PREFERENCE;

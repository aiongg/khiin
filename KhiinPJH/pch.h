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

#include <ctffunc.h>
#include <msctf.h>
#include <windows.h>
#include <winrt/base.h>

#pragma comment(lib, "windowsapp")

#include "log.h"
#include "resource.h"

namespace winrt {

// We need this so that we can implement only ITfTextInputProcessorEx
// and tell winrt that we also support ITfTextProcessorInput
// https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/author-coclasses#implement-a-com-interface-that-derives-from-another
template <>
inline bool is_guid_of<ITfTextInputProcessorEx>(guid const &id) noexcept {
    return is_guid_of<ITfTextInputProcessorEx, ITfTextInputProcessor>(id);
}

} // namespace winrt

#define DELETE_COPY_AND_ASSIGN(TypeName)                                                                               \
  public:                                                                                                              \
    TypeName() = default;                                                                                              \
    TypeName(const TypeName &) = delete;                                                                               \
    TypeName &operator=(const TypeName &) = delete;                                                                    \
    ~TypeName() = default;
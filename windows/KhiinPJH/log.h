#pragma once

#ifdef _DEBUG
#include <comdef.h>
#include <iomanip>
#include <sstream>

#define D(...) LogDebugStringTmp(__LINE__, __FILE__, __VA_ARGS__)

template <typename... Args>
void LogDebugStringTmp(int line, const char *fileName, Args &&...args) {
    auto fname = std::string(fileName);
    auto wfname = std::wstring(fname.cbegin(), fname.cend());
    std::wostringstream stream;
    stream << wfname << L"(" << line << L") : ";
    (stream << ... << std::forward<Args>(args)) << std::endl;
    auto str = stream.str();
    auto wstr = std::wstring(str.cbegin(), str.cend());

    ::OutputDebugString(wstr.c_str());
}

template <typename... Args>
void LogHresultError(HRESULT hr, int line, const char *filename) {
    auto e = _com_error(hr);
    auto fname = std::string(filename);
    auto wfname = std::wstring(fname.cbegin(), fname.cend());
    std::wostringstream stream;
    stream << wfname << L"(" << line << L") : 0x" << std::hex << hr << " (" << e.ErrorMessage() << ")" << std::endl;
    auto str = stream.str();
    auto wstr = std::wstring(str.cbegin(), str.cend());

    ::OutputDebugString(wstr.c_str());
}

#define CHECK_HRESULT(hr)                        \
    if (FAILED(hr)) {                            \
        LogHresultError(hr, __LINE__, __FILE__); \
    }

#define CHECK_RETURN_HRESULT(hr)                 \
    if (FAILED(hr)) {                            \
        LogHresultError(hr, __LINE__, __FILE__); \
        return hr;                               \
    }

#else
#define D(...)
#define CHECK_RETURN_HRESULT(hr)
#endif

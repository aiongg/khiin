#pragma once

#include <filesystem>

namespace khiin::win32 {

struct Utils {
    static const std::wstring Widen(const std::string &str);
    static const std::string Narrow(const std::wstring &wstr);
};

} // namespace khiin::win32

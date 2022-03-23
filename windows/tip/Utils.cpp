#include "pch.h"

#include "Utils.h"

#include <utf8cpp/utf8/cpp17.h>

namespace khiin::win32 {

std::wstring const Utils::Widen(const std::string &str) {
    auto tmp = utf8::utf8to16(str);
    return std::wstring(tmp.cbegin(), tmp.cend());
}

std::string const Utils::Narrow(const std::wstring &str) {
    auto tmp = std::u16string(str.cbegin(), str.cend());
    return utf8::utf16to8(tmp);
}

} // namespace khiin::win32

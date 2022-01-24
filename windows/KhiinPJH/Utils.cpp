#include "pch.h"

#include "Utils.h"

#include <utf8cpp/utf8/cpp17.h>

namespace khiin::win32 {

using namespace messages;

std::wstring const Utils::Widen(const std::string &str) {
    auto tmp = utf8::utf8to16(str);
    return std::wstring(tmp.cbegin(), tmp.cend());
}

std::string const Utils::Narrow(const std::wstring &str) {
    auto tmp = std::u16string(str.cbegin(), str.cend());
    return utf8::utf16to8(tmp);
}

WidePreedit const Utils::WidenPreedit(const Preedit &preedit) {
    auto ret = WidePreedit{};
    auto &segments = preedit.segments();
    auto start_idx = 0;
    for (auto &segment : segments) {
        auto w = Widen(segment.value());
        ret.preedit_display += w;
        ret.segment_start_and_size.push_back(std::make_pair(start_idx, w.size()));
        ret.segment_status.push_back(segment.status());
        start_idx += w.size();
    }

    // Not sure if this will work with 4-byte UTF-16 strings (e.g. Hanji)
    ret.cursor = preedit.cursor_position();
    ret.display_size = ret.preedit_display.size();
    return ret;
}

} // namespace khiin::win32

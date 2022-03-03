#pragma once

#include "common.h"

#include <filesystem>

namespace khiin::win32 {

struct WidePreedit {
    long cursor = 0;
    long display_size = 0;
    std::wstring preedit_display;
    std::vector<std::pair<long, long>> segment_start_and_size;
    std::vector<messages::SegmentStatus> segment_status;
};

struct Utils {
    static const std::wstring Widen(const std::string &str);
    static const std::string Narrow(const std::wstring &wstr);
    static WidePreedit const WidenPreedit(const messages::Preedit &preedit);
};

} // namespace khiin::win32

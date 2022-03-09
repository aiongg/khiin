#pragma once

#include "common.h"

#include <filesystem>

namespace khiin::win32 {

struct WidePreedit {
    int cursor = 0;
    int display_size = 0;
    std::wstring preedit_display;
    std::vector<std::pair<int, int>> segment_start_and_size;
    std::vector<proto::SegmentStatus> segment_status;
};

struct Utils {
    static const std::wstring Widen(const std::string &str);
    static const std::string Narrow(const std::wstring &wstr);
    static WidePreedit const WidenPreedit(const proto::Preedit &preedit);
};

} // namespace khiin::win32

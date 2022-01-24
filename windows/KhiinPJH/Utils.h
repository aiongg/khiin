#pragma once

namespace khiin::win32 {

struct Utils {
    static const std::wstring Widen(std::string_view);
    static const std::string Narrow(std::wstring_view);
};

}

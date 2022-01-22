#pragma once

namespace khiin::win32 {

struct Utils {
    static std::wstring Widen(std::string_view);
    static std::string Narrow(std::wstring_view);
};

}

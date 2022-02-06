#include "unicode_utils.h"

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>
#include <utf8cpp/utf8/cpp17.h>

namespace khiin::engine::unicode {

std::string to_nfd(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);
    return utf8::utf32to8(u32s);
}

std::string to_nfc(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfc(u32s);
    return utf8::utf32to8(u32s);
}

std::string strip_diacritics(std::string_view str, bool strip_letter_diacritics) {
    auto u32s = utf8::utf8to32(to_nfd(str));
    auto stripped = std::u32string();
    char32_t from = 0x0300;
    char32_t to = 0x030d;

    if (strip_letter_diacritics) {
        to = 0x0358;
    }

    for (auto &c : u32s) {
        if (from <= c && c <= to) {
            continue;
        }

        stripped.push_back(c);
    }

    return utf8::utf32to8(stripped);
}

bool starts_with_alnum(std::string_view str) {
    if (str.empty()) {
        return false;
    }

    auto nfd = to_nfd(str);
    return isalnum(nfd.front());
}

bool ends_with_alnum(std::string_view str) {
    if (str.empty()) {
        return false;
    }

    auto stripped = strip_diacritics(str, true);

    return isalnum(stripped.back());
}

} // namespace khiin::engine

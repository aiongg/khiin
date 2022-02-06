#pragma once

#include <utf8cpp/utf8.h>

namespace khiin::engine {

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

namespace unicode {

template <typename T>
static inline utf8_size_t utf8_size(T string) {
    return static_cast<utf8_size_t>(utf8::distance(string.cbegin(), string.cend()));
}

std::string to_nfd(std::string_view str);
std::string to_nfc(std::string_view str);
std::string strip_diacritics(std::string_view str, bool strip_letter_diacritics = false);

bool starts_with_alnum(std::string_view str);
bool ends_with_alnum(std::string_view str);

} // namespace unicode
} // namespace khiin::engine
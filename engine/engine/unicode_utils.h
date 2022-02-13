#pragma once

#include <utf8cpp/utf8.h>

namespace khiin::engine {

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

namespace unicode {

inline constexpr char32_t kHanjiCutoff = 0x2e80;

enum class GlyphCategory {
    Other,
    Alnum,
    Khin,
    Hanji,
};

template <typename T>
static inline utf8_size_t utf8_size(T string) {
    return static_cast<utf8_size_t>(utf8::distance(string.cbegin(), string.cend()));
}

std::string to_nfd(std::string_view str);
std::string to_nfc(std::string_view str);
std::string strip_diacritics(std::string_view str, bool strip_letter_diacritics = false);

GlyphCategory glyph_type(std::string::const_iterator const &it);
GlyphCategory start_glyph_type(std::string_view str);
GlyphCategory end_glyph_type(std::string_view str);

size_t letter_count(std::string input);

inline bool contains_hanji(std::string_view str) {
    auto it = str.cbegin();
    auto end = str.cend();
    while (it != end) {
        auto cp = utf8::unchecked::next(it);
        if (cp >= kHanjiCutoff) {
            return true;
        }
    }
    return false;
}

} // namespace unicode
} // namespace khiin::engine
#include "unicode_utils.h"

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>

namespace khiin::unicode {

namespace {

GlyphCategory glyph_category_of_codepoint(char32_t cp) {
    if (cp <= 0xFF && isalnum(cp)) {
        return GlyphCategory::Alnum;
    }

    if (cp == 0x00b7) {
        return GlyphCategory::Khin;
    }

    if (cp >= kHanjiCutoff) {
        return GlyphCategory::Hanji;
    }

    return GlyphCategory::Other;
}

} // namespace

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

GlyphCategory glyph_type(std::string::const_iterator const &it) {
    auto cp = utf8::unchecked::peek_next(it);
    return glyph_category_of_codepoint(cp);
}

GlyphCategory start_glyph_type(std::string_view str) {
    if (str.empty()) {
        return GlyphCategory::Other;
    }

    auto nfd = to_nfd(str);
    auto it = nfd.begin();
    auto cp = utf8::unchecked::peek_next(it);
    return glyph_category_of_codepoint(cp);
}

GlyphCategory end_glyph_type(std::string_view str) {
    if (str.empty()) {
        return GlyphCategory::Other;
    }

    auto stripped = strip_diacritics(str, true);
    auto size = utf8::unchecked::distance(stripped.begin(), stripped.end());
    auto it = stripped.begin();
    utf8::unchecked::advance(it, size - 1);
    auto cp = utf8::unchecked::peek_next(it);
    return glyph_category_of_codepoint(cp);
}

} // namespace khiin::engine::unicode

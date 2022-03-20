#pragma once

#include <algorithm>

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>
#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

namespace khiin {

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

namespace unicode {

inline constexpr char kMaxAscii = 0x7f;
inline constexpr char32_t kHanjiCutoff = 0x2e80;
inline constexpr char32_t kNasalLower = 0x207f;
inline constexpr char32_t kNasalUpper = 0x1d3a;
inline constexpr char32_t kDotAboveRight = 0x0358;
inline constexpr char32_t kDotsBelow = 0x0324;
inline constexpr char32_t kKhinDot = 0x00b7;
inline constexpr char32_t kTone2 = 0x0301;
inline constexpr char32_t kTone3 = 0x0300;
inline constexpr char32_t kTone5 = 0x0302;
inline constexpr char32_t kTone7 = 0x0304;
inline constexpr char32_t kTone8 = 0x030d;
inline constexpr char32_t kTone9 = 0x0306;
inline constexpr char32_t kToneLowerBound = kTone3;
inline constexpr char32_t kToneUpperBound = kTone8;

inline constexpr bool is_tone(char32_t c) {
    return c >= kToneLowerBound && c <= kToneUpperBound;
}

inline constexpr bool is_hanji(char32_t c) {
    return c >= kHanjiCutoff;
}

inline constexpr bool is_nasal(char32_t c) {
    return c == kNasalLower || c == kNasalUpper;
}

inline constexpr bool is_khin(char32_t c) {
    return c == kKhinDot;
}

enum class GlyphCategory {
    Other,
    Alnum,
    AsciiPunct,
    Khin,
    Hanji,
};

namespace {

const inline GlyphCategory glyph_category_of_codepoint(char32_t cp) {
    if ((cp <= kMaxAscii && isalnum(cp)) || is_nasal(cp)) {
        return GlyphCategory::Alnum;
    }

    if (cp <= kMaxAscii && ispunct(cp)) {
        return GlyphCategory::AsciiPunct;
    }

    if (cp == 0x00b7) {
        return GlyphCategory::Khin;
    }

    if (is_hanji(cp)) {
        return GlyphCategory::Hanji;
    }

    return GlyphCategory::Other;
}

} // namespace

template <typename IterT>
using u8iterator = utf8::unchecked::iterator<IterT>;

template <typename StrT>
static inline utf8_size_t u8_size(StrT const &string) {
    return static_cast<utf8_size_t>(utf8::unchecked::distance(string.cbegin(), string.cend()));
}

template <typename StrT>
std::string to_nfd(StrT const &s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);
    return utf8::utf32to8(u32s);
}

template <typename StrT>
std::string to_nfc(StrT const &s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfc(u32s);
    return utf8::utf32to8(u32s);
}

inline std::u32string u8_to_u32_nfd(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);
    return u32s;
}

inline std::string u32_to_u8_nfc(std::u32string &u32str) {
    ufal::unilib::uninorms::nfc(u32str);
    return utf8::utf32to8(u32str);
}

std::string strip_diacritics(std::string_view str, bool strip_letter_diacritics = false);

template <typename octet_iterator>
GlyphCategory glyph_type(octet_iterator it) {
    auto cp = utf8::unchecked::peek_next(it);
    return glyph_category_of_codepoint(cp);
}

GlyphCategory start_glyph_type(std::string_view str);

GlyphCategory end_glyph_type(std::string_view str);

template <typename StrT>
utf8::unchecked::iterator<typename StrT::iterator> u8_begin(StrT &str) {
    return utf8::unchecked::iterator<typename StrT::iterator>(str.begin());
}

template <typename StrT>
utf8::unchecked::iterator<typename StrT::iterator> u8_end(StrT &str) {
    return utf8::unchecked::iterator<typename StrT::iterator>(str.end());
}

template <typename StrT>
utf8::unchecked::iterator<typename StrT::const_iterator> u8_cbegin(StrT const &str) {
    return utf8::unchecked::iterator<typename StrT::const_iterator>(str.cbegin());
}

template <typename StrT>
utf8::unchecked::iterator<typename StrT::const_iterator> u8_cend(StrT const &str) {
    return utf8::unchecked::iterator<typename StrT::const_iterator>(str.cend());
}

template <typename StrT>
bool contains_hanji(StrT str) {
    auto it = u8_cbegin(str);
    auto end = u8_cend(str);

    for (; it != end; ++it) {
        if (*it >= kHanjiCutoff) {
            return true;
        }
    }

    return false;
}

template <typename StrT>
bool contains_only_hanji(StrT str) {
    auto it = u8_cbegin(str);
    auto end = u8_cend(str);

    for (; it != end; ++it) {
        if (*it < kHanjiCutoff) {
            return false;
        }
    }

    return true;
}

template <typename StrT>
size_t letter_count(StrT input) {
    auto nfd = to_nfd(input);
    auto count = 0;
    auto it = u8_cbegin(nfd);
    auto end = u8_cend(nfd);
    for (; it != end; ++it) {
        if (*it < 0xff && isalpha(*it)) {
            ++count;
        }
    }
    return count;
}

template <typename StrT>
void str_tolower(StrT &str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
}

template <typename StrT>
std::string copy_str_tolower(StrT const &str) {
    auto ret = std::string();
    std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), [](unsigned char c) {
        return std::tolower(c);
    });
    return ret;
}

template <typename StrT>
bool all_lower(StrT const &str) {
    auto nfd = to_nfd(str);
    auto it = u8_cbegin(nfd);
    auto end = u8_cend(nfd);
    for (; it != end; ++it) {
        if (*it <= kMaxAscii && isupper(*it)) {
            return false;
        }
    }
    return true;
}

inline void safe_erase(std::string &str, utf8_size_t index, size_t count = 1) {
    auto size = u8_size(str);
    if (index >= size) {
        return;
    }
    auto from = str.begin();
    utf8::unchecked::advance(from, index);
    auto to = from;
    utf8::unchecked::advance(to, count);
    str.erase(from, to);
}

} // namespace unicode
} // namespace khiin
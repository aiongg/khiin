#pragma once

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>
#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

namespace khiin {

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

namespace unicode {

inline constexpr char32_t kHanjiCutoff = 0x2e80;

enum class GlyphCategory {
    Other,
    Alnum,
    Khin,
    Hanji,
};

namespace {

const inline GlyphCategory glyph_category_of_codepoint(char32_t cp) {
    if ((cp <= 0xFF && isalnum(cp)) || cp == 0x207f) {
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
    return utf8::unchecked::iterator<StrT::iterator>(str.begin());
}

template <typename StrT>
utf8::unchecked::iterator<typename StrT::iterator> u8_end(StrT &str) {
    return utf8::unchecked::iterator<StrT::iterator>(str.end());
}

template <typename StrT>
utf8::unchecked::iterator<typename StrT::const_iterator> u8_cbegin(StrT const &str) {
    return utf8::unchecked::iterator<StrT::const_iterator>(str.cbegin());
}

template <typename StrT>
utf8::unchecked::iterator<typename StrT::const_iterator> u8_cend(StrT const &str) {
    return utf8::unchecked::iterator<StrT::const_iterator>(str.cend());
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
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

#include "lomaji.h"

namespace utf8u = utf8::unchecked;

namespace TaiKey {

// Utility methods (not exposed)

auto static asciiLettersPerCodepoint(uint32_t cp) {
    if (cp == ' ' || (0x0300 <= cp && cp <= 0x030d)) {
        return 0;
    } else if (cp == 0x207f || cp == 0x1d3a || cp == 0x1e72 || cp == 0x1e73) {
        return 2;
    }

    return 1;
}

auto static cursorSkipsCodepoint(uint32_t cp) {
    return 0x0300 <= cp && cp <= 0x0358;
}

auto getToneFromKeyMap(std::unordered_map<Tone, char> map, char ch) {
    for (const auto &it : map) {
        if (it.second == ch) {
            return it.first;
        }
    }

    return Tone::NaT;
}

auto stripToAlpha(std::string str) {
    static boost::regex re("[\\d]+");
    return boost::replace_all_copy(boost::regex_replace(str, re, ""), "-", " ");
}

// Exposed methods

/**
 * Parameter @ascii may have:
 *     [--]letters[1-9][0][-]
 * - A trailing tone digit, trailing 0 for khin, and trailing hyphen
 * - Khin double-dash prefix
 */
auto asciiSyllableToUtf8(std::string ascii) -> std::string {
    bool khin = false;
    Tone tone = Tone::NaT;
    bool trailingHyphen = false;

    auto end = ascii.cend();
    auto begin = ascii.cbegin();
    --end;

    if (*end == '-') {
        trailingHyphen = true;
        --end;
    }

    if (*end == '0') {
        khin = true;
        --end;
    }

    if (isdigit(*end)) {
        tone = getToneFromDigit(*end);
        --end;
    }

    if (ascii.rfind("--", 0) == 0) {
        khin = true;
        begin += 2;
    }

    auto utf8 = asciiSyllableToUtf8(std::string(begin, ++end), tone, khin);

    if (trailingHyphen) {
        utf8.insert(utf8.end(), '-');
    }

    return utf8;
}

// ascii must not have a tone number (digit)
auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin)
    -> std::string {
    std::string ret = ascii;

    boost::algorithm::replace_first(ret, "nn", U8_NN);
    boost::algorithm::replace_first(ret, "ou", "o" + U8_OU);
    boost::algorithm::replace_first(ret, "oo", "o" + U8_OU);
    boost::algorithm::replace_first(ret, "ur", U8_UR);
    boost::algorithm::replace_first(ret, "or", "o" + U8_R);

    if (khin) {
        ret.insert(0, U8_TK);
    }

    if (tone != Tone::NaT) {
        ret = placeToneOnSyllable(ret, tone);
    }

    return toNFC(ret);
}

auto checkTone78Swap(std::string u8syllable, Tone tone) -> Tone {
    if (u8syllable.empty() || (tone != Tone::T7 && tone != Tone::T8)) {
        return tone;
    }

    auto end = u8syllable.back();

    if ((PTKH.find(end) == PTKH.end()) && tone == Tone::T8) {
        return Tone::T7;
    } else if ((PTKH.find(end) != PTKH.end() && tone == Tone::T7)) {
        return Tone::T8;
    }

    return tone;
}

auto getAsciiCursorFromUtf8(std::string ascii, std::string u8str, size_t idx)
    -> size_t {
    auto u8_it = u8str.begin();
    auto u8_target = u8str.begin();
    auto u8_end = u8str.end();
    auto a_it = ascii.begin();
    auto ucp = uint32_t(0);
    auto acp = uint32_t(0);

    if (idx >= static_cast<size_t>(utf8u::distance(u8_it, u8_end))) {
        u8_target = u8_end;
    } else {
        utf8u::advance(u8_target, idx);
    }

    try {
        while (u8_it != u8_target) {
            ucp = utf8u::peek_next(u8_it);

            if (U32_T3 <= ucp && ucp <= U32_T8) {
                utf8u::advance(u8_it, 1);
            } else if (ucp == U32_OU) {
                utf8u::advance(a_it, 1);
                utf8u::advance(u8_it, 1);
            } else if (ucp == U32_NN) {
                utf8u::advance(a_it, 2);
                utf8u::advance(u8_it, 1);
            } else if (ucp == U32_TK) {
                utf8u::advance(u8_it, 1);
            } else {
                utf8u::advance(a_it, 1);
                utf8u::advance(u8_it, 1);
            }
        }

        return static_cast<size_t>(utf8u::distance(ascii.begin(), a_it));
    } catch (const utf8::not_enough_room &e) {
        return static_cast<size_t>(utf8u::distance(ascii.begin(), a_it));
    }
}

auto getToneFromDigit(char ch) -> Tone {
    return getToneFromKeyMap(ToneToDigitMap, ch);
}

auto getToneFromTelex(char ch) -> Tone {
    return getToneFromKeyMap(ToneToTelexMap, ch);
}

auto hasToneDiacritic(std::string str) -> bool {
    // boost regex does not support unicode unless we use ICU version
    str = toNFD(str);

    auto b = str.cbegin();
    auto e = str.cend();
    auto b_s = U32_TONES.cbegin();
    auto e_s = U32_TONES.cend();

    while (b != e) {
        if (std::find(b_s, e_s, utf8u::peek_next(b)) != e_s) {
            return true;
        }
        utf8u::advance(b, 1);
    }

    return false;
}

auto parallelNext(std::string::iterator &a_it, std::string::iterator &a_end,
                  std::string::iterator &u_it, std::string::iterator &u_end)
    -> void {
    while (u_it != u_end && (utf8::peek_next(u_it, u_end) == '-' ||
                             utf8::peek_next(u_it, u_end) == ' ')) {
        utf8::advance(u_it, 1, u_end);

        if (a_it != a_end && a_it + 1 != a_end && *(a_it + 1) == '-') {
            a_it++;
        }
    }

    while (a_it != a_end && a_it + 1 != a_end && isdigit(*(a_it + 1))) {
        a_it++;
    }

    if (u_it == u_end || a_it == a_end) {
        return;
    }

    auto cp = utf8::next(u_it, u_end);

    a_it += asciiLettersPerCodepoint(cp);

    while (u_it != u_end &&
           cursorSkipsCodepoint(utf8::peek_next(u_it, u_end))) {
        cp = utf8::next(u_it, u_end);
        a_it += asciiLettersPerCodepoint(cp);
    }

    while (a_it != a_end && isdigit(*a_it)) {
        a_it++;
    }
}

auto parallelPrior(std::string::iterator &a_it, std::string::iterator &a_begin,
                   std::string::iterator &u_it, std::string::iterator &u_begin)
    -> void {
    if (a_it == a_begin || u_it == u_begin) {
        return;
    }

    while (a_it != a_begin && isdigit(*(a_it - 1))) {
        a_it--;
    }

    auto cp = utf8::prior(u_it, u_begin);

    while (u_it != u_begin && cursorSkipsCodepoint(cp)) {
        a_it -= asciiLettersPerCodepoint(cp);
        cp = utf8::prior(u_it, u_begin);
    }

    if (cp == U32_TK && std::distance(a_begin, a_it) > 1 &&
        *(a_it - 1) == '-' && *(a_it - 2) == '-') {
        a_it -= 2;
    } else {
        a_it -= asciiLettersPerCodepoint(cp);
    }
}

auto stripDiacritics(std::string str) {

    auto u32s = utf8::utf8to32(str);
    auto stripped = std::u32string();

    for (auto &c : u32s) {
        stripped.push_back(ufal::unilib::unistrip::strip_combining_marks(c));
    }

    return boost::erase_all_copy(utf8::utf32to8(stripped), u8"\u00b7");
}

auto placeToneOnSyllable(std::string u8syllable, Tone tone) -> std::string {
    if (tone == Tone::NaT) {
        return u8syllable;
    }

    static boost::regex tone_mid_ae(u8"o[ae][mnptkh]");
    static std::string ordered_vowel_matches[] = {u8"o", u8"a",  u8"e", u8"u",
                                                  u8"i", u8"ng", u8"m"};

    auto ret = stripDiacritics(u8syllable);

    // BOOST_LOG_TRIVIAL(debug) << boost::format("stripped syl: %1%") % ret;

    boost::smatch match;
    ptrdiff_t found;

    if (boost::regex_search(ret, match, tone_mid_ae)) {
        found = match.position() + 1;
    } else {
        for (std::string v : ordered_vowel_matches) {
            found = ret.find(v);

            if (found != std::string::npos) {
                break;
            }
        }
    }

    if (found == std::string::npos) {
        return u8syllable;
    }

    ret.insert(found + 1, ToneToUtf8Map.at(tone));

    return ret;
}

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji) -> VStr {
    // auto ret = VStr();

    auto segments = VStr();
    auto base = stripToAlpha(utf8ToAsciiLower(lomaji));

    auto a_start = ascii.begin();
    auto a_it = ascii.begin();
    auto a_end = ascii.end();

    auto b_it = base.begin();
    auto b_end = base.end();

    while (a_it != a_end && b_it != b_end) {
        if (*b_it == ' ') {
            while (a_it != a_end && (isdigit(*a_it) || *a_it == '-')) {
                ++a_it;
            }

            segments.push_back(std::string(a_start, a_it));
            a_start = a_it;
            ++b_it;
        } else {
            while (*a_it == '-') {
                ++a_it;
            }
            ++a_it;
            ++b_it;
        }
    }

    segments.push_back(std::string(a_start, a_end));
    return segments;
}

auto toNFD(std::string_view s) -> std::string {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);

    return utf8::utf32to8(u32s);
}

auto toNFC(std::string_view s) -> std::string {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfc(u32s);

    return utf8::utf32to8(u32s);
}

auto utf8Size(std::string s) -> Utf8Size {
    return static_cast<Utf8Size>(utf8::distance(s.begin(), s.end()));
}

auto utf8ToAsciiLower(std::string u8string) -> std::string {
    boost::algorithm::trim_if(
        u8string, [](char &c) -> bool { return c == ' ' || c == '-'; });

    if (u8string.empty()) {
        return u8string;
    }

    u8string = toNFD(u8string);
    auto start = u8string.begin();
    auto it = u8string.begin();
    auto end = u8string.end();
    auto u8_cp = uint32_t(0);

    while (it != u8string.end()) {
        start = it;
        u8_cp = utf8::next(it, end);
        if (ToneUint32ToDigitMap.find(u8_cp) != ToneUint32ToDigitMap.end()) {
            auto &repl = ToneUint32ToDigitMap.at(u8_cp);
            u8string.replace(start, it, repl);
            end = u8string.end();
            it = start;
            std::advance(it, repl.size());
        }
    }

    boost::algorithm::to_lower(u8string);

    static boost::regex sylWithMidNumericTone("([a-z]+)(\\d)([a-z]+)");
    static boost::regex khinAtFront("(0)([a-z]+\\d?)");
    u8string = boost::regex_replace(u8string, sylWithMidNumericTone, "$1$3$2");
    u8string = boost::regex_replace(u8string, khinAtFront, "$2$1");

    return u8string;
}

} // namespace TaiKey

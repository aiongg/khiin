#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

#include "lomaji.h"

namespace utf8u = utf8::unchecked;

namespace khiin::engine {

namespace {

auto getToneFromKeyMap(std::unordered_map<Tone, char> map, char ch) {
    for (const auto &it : map) {
        if (it.second == ch) {
            return it.first;
        }
    }

    return Tone::NaT;
}

auto stripToAlpha(std::string str) {
    static std::regex re("[\\d]+");
    return boost::replace_all_copy(std::regex_replace(str, re, ""), "-", " ");
}

} // namespace

/**
 * Parameter @ascii may have:
 *     [--]letters[1-9][0][-]
 * - A trailing tone digit, trailing 0 for khin, and trailing hyphen
 * - Khin double-dash prefix
 */
auto asciiSyllableToUtf8(std::string ascii) -> std::string {
    if (ascii.size() < 2) {
        return ascii;
    }

    if (ascii == u8"--") {
        return U8_TK;
    }

    bool khin = false;
    Tone tone = Tone::NaT;
    bool trailingHyphen = false;

    auto begin = ascii.cbegin();
    auto back = ascii.cend() - 1;

    if (ascii.rfind("--", 0) == 0) {
        khin = true;
        begin += 2;
    }

    if (*back == '-') {
        trailingHyphen = true;
        --back;
    }

    if (*back == '0') {
        khin = true;
        --back;
    }

    if (isdigit(*back)) {
        tone = getToneFromDigit(*back);
        --back;
    }

    auto utf8 = asciiSyllableToUtf8(std::string(begin, ++back), tone, khin);

    if (trailingHyphen) {
        utf8.insert(utf8.end(), '-');
    }

    return utf8;
}

// ascii must not have a tone number (digit)
auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin) -> std::string {
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

auto getToneFromDigit(char ch) -> Tone {
    return getToneFromKeyMap(ToneToDigitMap, ch);
}

auto getToneFromTelex(char ch) -> Tone {
    return getToneFromKeyMap(ToneToTelexMap, ch);
}

auto hasToneDiacritic(std::string str) -> bool {
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

    static std::regex tone_mid_ae(u8"o[ae][mnptkh]");
    static std::string ordered_vowel_matches[] = {u8"o", u8"a", u8"e", u8"u", u8"i", u8"ng", u8"m"};

    auto ret = stripDiacritics(u8syllable);

    // BOOST_LOG_TRIVIAL(debug) << boost::format("stripped syl: %1%") % ret;

    std::smatch match;
    ptrdiff_t found;

    if (std::regex_search(ret, match, tone_mid_ae)) {
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

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji) -> string_vector {
    auto segments = string_vector();
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

auto tokenSpacer(std::vector<std::string_view> tokens) -> std::vector<bool> {
    auto ret = std::vector<bool>();
    ret.reserve(tokens.size());

    if (tokens.size() == 1) {
        ret.emplace_back(false);
        return ret;
    }

    for (auto it = tokens.cbegin(); it != tokens.cend() - 1; ++it) {
        auto lcp = utf8back(it[0]);
        auto rcp = utf8first(it[1]);

        // add virtual space if either side has lomaji and
        // left side doesn't have hyphen
        if (lcp >= 0x2e80 && rcp >= 0x2e80) {
            ret.push_back(false);
        } else if (lcp != '-') {
            ret.push_back(true);
        }
    }
    ret.push_back(false);
    return ret;
}

auto toNFD(std::string_view s) -> std::string {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);
    return std::move(utf8::utf32to8(u32s));
}

auto toNFC(std::string_view s) -> std::string {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfc(u32s);
    return std::move(utf8::utf32to8(u32s));
}

auto utf8back(std::string_view str) -> uint32_t {
    auto end = str.cend();
    return utf8::prior(end, str.cbegin());
}

auto utf8first(std::string_view str) -> uint32_t {
    return utf8::peek_next(str.cbegin(), str.cend());
}

auto utf8Size(std::string s) -> utf8_size_t {
    return static_cast<utf8_size_t>(utf8::distance(s.begin(), s.end()));
}

auto utf8ToAsciiLower(std::string u8string) -> std::string {
    boost::algorithm::trim_if(u8string, [](char &c) -> bool {
        return c == ' ' || c == '-';
    });

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
        u8_cp = utf8u::next(it);
        if (ToneUint32ToDigitMap.find(u8_cp) != ToneUint32ToDigitMap.end()) {
            auto &repl = ToneUint32ToDigitMap.at(u8_cp);
            u8string.replace(start, it, repl);
            end = u8string.end();
            it = start;
            std::advance(it, repl.size());
        }
    }

    boost::algorithm::to_lower(u8string);

    static std::regex sylWithMidNumericTone("([a-z]+)(\\d)([a-z]+)");
    static std::regex khinAtFront("(0)([a-z]+\\d?)");
    u8string = std::regex_replace(u8string, sylWithMidNumericTone, "$1$3$2");
    u8string = std::regex_replace(u8string, khinAtFront, "$2$1");

    return u8string;
}

} // namespace khiin::engine
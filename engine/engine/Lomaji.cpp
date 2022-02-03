#include <regex>

#include <boost/algorithm/string.hpp>
//#include <boost/format.hpp>
//#include <boost/log/trivial.hpp>

#include <unilib/uninorms.h>
#include <unilib/unistrip.h>

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

#include "Lomaji.h"

namespace utf8u = utf8::unchecked;

namespace khiin::engine {

namespace {

inline const std::string U8_NN = u8"\u207f";
inline const std::string U8_OU = u8"\u0358";
inline const std::string U8_T2 = u8"\u0301";
inline const std::string U8_T3 = u8"\u0300";
inline const std::string U8_T5 = u8"\u0302";
inline const std::string U8_T7 = u8"\u0304";
inline const std::string U8_T8 = u8"\u030D";
inline const std::string U8_T9 = u8"\u0306";
inline const std::string U8_TK = u8"\u00b7";
inline const std::string U8_R = u8"\u0324";
inline const std::string U8_UR = u8"\u1e73";

inline const std::unordered_set<uint32_t> TwoAsciiCodepoints = {U32_TK, U32_NN, U32_NN_UC, U32_UR, U32_UR_UC};

inline const std::unordered_map<Tone, char> ToneToTelexMap = {{Tone::T2, 's'}, {Tone::T3, 'f'}, {Tone::T5, 'l'},
                                                              {Tone::T7, 'j'}, {Tone::T8, 'j'}, {Tone::T9, 'w'},
                                                              {Tone::TK, 'q'}};

inline const std::unordered_map<Tone, char> ToneToDigitMap = {{Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'},
                                                              {Tone::T7, '7'}, {Tone::T8, '8'}, {Tone::T9, '9'},
                                                              {Tone::TK, '0'}};

inline const std::unordered_map<Tone, std::string> ToneToUtf8Map = {
    {Tone::T2, U8_T2}, {Tone::T3, U8_T3}, {Tone::T5, U8_T5}, {Tone::T7, U8_T7},
    {Tone::T8, U8_T8}, {Tone::T9, U8_T9}, {Tone::TK, U8_TK}};

inline static std::unordered_map<uint32_t, std::string> ToneUint32ToDigitMap = {
    {U32_T2, "2"}, {U32_T3, "3"}, {U32_T5, "5"}, {U32_T7, "7"}, {U32_T8, "8"},
    {U32_T9, "9"}, {U32_TK, "0"}, {U32_R, "r"},  {U32_OU, "u"}, {U32_NN, "nn"}};

inline const std::unordered_map<Tone, uint32_t> ToneToUint32Map = {
    {Tone::T2, U32_T2}, {Tone::T3, U32_T3}, {Tone::T5, U32_T5}, {Tone::T7, U32_T7},
    {Tone::T8, U32_T8}, {Tone::T9, U32_T9}, {Tone::TK, U32_TK}};

inline const std::string TONES[] = {U8_T2, U8_T3, U8_T5, u8"\u030c", U8_T7, U8_T8, u8"\u0306", U8_TK};

inline const auto U32_TONES = std::vector<uint32_t>{U32_T2, U32_T3, U32_T5, U32_T7, U32_T8, U32_T9};

inline const std::unordered_set<char> PTKH = {'P', 'T', 'K', 'H', 'p', 't', 'k', 'h'};

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

bool IsNoncursorableCodepoint(uint32_t cp) {
    return 0x0300 < cp && cp < 0x0358;
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

utf8_size_t Lomaji::MoveCaret(std::string_view str, utf8_size_t start_pos, CursorDirection dir) {
    auto it = str.cbegin();

    if (dir == CursorDirection::L) {
        if (start_pos == 0) {
            return 0;
        }

        utf8::unchecked::advance(it, start_pos);

        while (it != str.cbegin()) {
            auto cp = utf8::unchecked::prior(it);
            if (IsNoncursorableCodepoint(cp)) {
                continue;
            } else {
                break;
            }
        }
    } else if (dir == CursorDirection::R) {
        if (start_pos == Utf8Size(str)) {
            return start_pos;
        }

        utf8::unchecked::advance(it, start_pos + 1);

        while (it != str.cend()) {
            auto cp = utf8::unchecked::peek_next(it);
            if (IsNoncursorableCodepoint(cp)) {
                utf8::unchecked::next(it);
                continue;
            } else {
                break;
            }
        }
    }

    return utf8::unchecked::distance(str.cbegin(), it);
}

} // namespace khiin::engine

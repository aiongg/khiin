#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>

#include <utf8cpp/utf8.h>

#include "lomaji.h"

namespace TaiKey {

auto getToneFromKeyMap(std::unordered_map<Tone, char> map, char ch) {
    for (const auto &it : map) {
        if (it.second == ch) {
            return it.first;
        }
    }

    return Tone::NaT;
}

auto getToneFromDigit(char ch) -> Tone {
    return getToneFromKeyMap(ToneToDigitMap, ch);
}

auto getToneFromTelex(char ch) -> Tone {
    return getToneFromKeyMap(ToneToTelexMap, ch);
}

auto toNFD(std::string s) -> std::string {
    return boost::locale::normalize(s, boost::locale::norm_nfd);
}

auto toNFC(std::string s) -> std::string {
    return boost::locale::normalize(s, boost::locale::norm_nfc);
}

auto utf8Size(std::string s) -> Utf8Size {
    return static_cast<Utf8Size>(utf8::distance(s.begin(), s.end()));
}

auto stripDiacritics(std::string str) {
    static boost::regex tones("[\u00b7\u0300-\u030d]");
    return boost::regex_replace(toNFD(str), tones, "");
}

auto stripToAlpha(std::string str) {
    static boost::regex re("[\\d]+");
    return boost::replace_all_copy(boost::regex_replace(str, re, ""), "-", " ");
}

// ascii may have a tone number (digit)
auto asciiSyllableToUtf8(std::string ascii) -> std::string {
    bool khin = false;
    Tone tone = Tone::NaT;

    if (ascii.back() == '0') {
        khin = true;
        ascii.pop_back();
    }

    if (isdigit(ascii.back())) {
        tone = getToneFromDigit(ascii.back());
        ascii.pop_back();
    }

    return asciiSyllableToUtf8(ascii, tone, khin);
}

// ascii must not have a tone number (digit)
auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin)
    -> std::string {
    std::string ret = ascii;

    boost::algorithm::replace_first(ret, "nn", U8_NN);
    boost::algorithm::replace_first(ret, "ou", "o" + U8_OU);
    boost::algorithm::replace_first(ret, "oo", "o" + U8_OU);

    if (khin) {
        ret.insert(0, U8_TK);
    }

    if (tone != Tone::NaT) {
        ret = placeToneOnSyllable(ret, tone);
    }

    return toNFC(ret);
}

str_iter advanceRet(str_iter &it, int n, str_iter &end) {
    utf8::advance(it, n, end);
    return std::move(it);
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
    auto a_end = ascii.end();
    auto ucp = uint32_t(0);
    auto acp = uint32_t(0);

    if (idx >= static_cast<size_t>(utf8::distance(u8_it, u8_end))) {
        u8_target = u8_end;
    } else {
        utf8::advance(u8_target, idx, u8_end);
    }

    try {
        while (u8_it != u8_target) {
            ucp = utf8::peek_next(u8_it, u8_end);

            if (U32_T3 <= ucp && ucp <= U32_T8) {
                utf8::advance(u8_it, 1, u8_end);
            } else if (ucp == U32_OU) {
                utf8::advance(a_it, 1, a_end);
                utf8::advance(u8_it, 1, u8_end);
            } else if (ucp == U32_NN) {
                utf8::advance(a_it, 2, a_end);
                utf8::advance(u8_it, 1, u8_end);
            } else if (ucp == 0x0b7 /* middle dot */) {
                utf8::advance(u8_it, 1, u8_end);
            } else {
                utf8::advance(a_it, 1, a_end);
                utf8::advance(u8_it, 1, u8_end);
            }
        }

        return static_cast<size_t>(utf8::distance(ascii.begin(), a_it));
    } catch (const utf8::not_enough_room &e) {
        return static_cast<size_t>(utf8::distance(ascii.begin(), a_it));
    }
}

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji) -> VStr {
    // auto ret = VStr();

    auto segments = VStr();
    auto base = stripToAlpha(utf8ToAsciiLower(lomaji));

    //if (!cmpAsciiToUtf8(ascii, lomaji)) {
    //    return segments;
    //}

    auto a_start = ascii.begin();
    auto a_it = ascii.begin();
    auto a_end = ascii.end();

    auto b_it = base.begin();
    auto b_end = base.end();

    while (a_it != a_end && b_it != b_end) {
        if (*b_it == ' ') {
            while (a_it != a_end && isdigit(*a_it)) {
                a_it++;
            }
            
            segments.push_back(std::string(a_start, a_it));
            a_start = a_it;

            if (a_it != a_end && *a_it == '-') {
                a_it++;
                segments.push_back(std::string(a_start, a_it));
                a_start = a_it;
            }

            b_it++;
        } else {
            a_it++;
            b_it++;
        }
    }

    segments.push_back(std::string(a_start, a_end));
    return segments;
}

} // namespace TaiKey

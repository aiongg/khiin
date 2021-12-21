#include "lomaji.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>

#include <utf8cpp/utf8.h>

namespace TaiKey {

auto getToneFromKeyMap(std::unordered_map<Tone, char> map, char ch) {
    for (const auto &it : map) {
        if (it.second == ch) {
            return it.first;
        }
    }

    return Tone::NaT;
}

auto getToneFromDigit(char ch) -> Tone { return getToneFromKeyMap(TONE_DIGIT_MAP, ch); }

auto getToneFromTelex(char ch) -> Tone { return getToneFromKeyMap(TONE_TELEX_MAP, ch); }

auto toNFD(std::string s) -> std::string {
    return boost::locale::normalize(s, boost::locale::norm_nfd);
}

auto toNFC(std::string s) -> std::string {
    return boost::locale::normalize(s, boost::locale::norm_nfc);
}

auto utf8Size(std::string s) -> size_t {
    return static_cast<size_t>(utf8::distance(s.begin(), s.end()));
}

auto stripDiacritics(std::string str) {
    std::string ret = toNFD(str);
    for (std::string tone : TONES) {
        size_t found = ret.find(tone);
        if (found != std::string::npos) {
            ret.erase(found, tone.size());
            break;
        }
    }
    return ret;
}

auto asciiToUtf8(std::string ascii, Tone tone, bool khin) -> std::string {
    std::string ret = ascii;

    boost::algorithm::replace_first(ret, u8"nn", U8_NN);
    boost::algorithm::replace_first(ret, u8"ou", "o" + U8_OU);
    boost::algorithm::replace_first(ret, u8"oo", "o" + U8_OU);

    if (khin) {
        ret.insert(0, U8_TK);
    }

    if (tone != Tone::NaT) {
        ret.pop_back();
        ret = placeToneOnSyllable(ret, tone);
    }

    return toNFC(ret);
}

auto utf8ToAsciiLower(std::string u8string) -> std::string {
    boost::algorithm::trim_if(
        u8string, [](char& c) -> bool { return c == ' ' || c == '-'; });
    
    if (u8string.empty()) {
        return u8string;
    }

    u8string = toNFD(u8string);
    boost::algorithm::replace_all(u8string, U8_NN, "nn");
    boost::algorithm::replace_all(u8string, U8_OU, "u");
    boost::algorithm::replace_all(u8string, U8_R, "r");
    boost::algorithm::replace_all(u8string, U8_T2, "2");
    boost::algorithm::replace_all(u8string, U8_T3, "3");
    boost::algorithm::replace_all(u8string, U8_T5, "5");
    boost::algorithm::replace_all(u8string, U8_T7, "7");
    boost::algorithm::replace_all(u8string, U8_T8, "8");
    boost::algorithm::replace_all(u8string, U8_T9, "9");
    boost::algorithm::replace_all(u8string, U8_TK, "0");
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

    BOOST_LOG_TRIVIAL(debug) << boost::format("stripped syl: %1%") % ret;

    boost::smatch match;
    ptrdiff_t found;

    if (regex_search(ret, match, tone_mid_ae)) {
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

    ret.insert(found + 1, TONE_UTF_MAP.at(tone));

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

            if (0x0300 <= ucp && ucp <= 0x030D) {
                utf8::advance(u8_it, 1, u8_end);
            } else if (ucp == 0x0358) {
                utf8::advance(a_it, 1, a_end);
                utf8::advance(u8_it, 1, u8_end);
            } else if (ucp == 0x207f) {
                utf8::advance(a_it, 2, a_end);
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

} // namespace TaiKey

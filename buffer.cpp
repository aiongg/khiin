
#include "buffer.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utf8cpp/utf8.h>

namespace TaiKey {

const std::unordered_map<Tone, char> TONE_TELEX_MAP = {
    {Tone::T2, 's'}, {Tone::T3, 'f'}, {Tone::T5, 'l'}, {Tone::T7, 'j'},
    {Tone::T8, 'j'}, {Tone::T9, 'w'}, {Tone::TK, 'q'}};

const std::unordered_map<Tone, char> TONE_DIGIT_MAP = {
    {Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'}, {Tone::T7, '7'},
    {Tone::T8, '8'}, {Tone::T9, '9'}, {Tone::TK, '0'}};

const std::unordered_map<Tone, std::string> TONE_UTF_MAP = {
    {Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"}, {Tone::T5, u8"\u0302"},
    {Tone::T6, u8"\u030c"}, {Tone::T7, u8"\u0304"}, {Tone::T8, u8"\u030d"},
    {Tone::T9, u8"\u0306"}, {Tone::TK, u8"\u00b7"}};

const std::string TONES[] = {u8"\u0301", u8"\u0300", u8"\u0302", u8"\u030c",
                             u8"\u0304", u8"\u030d", u8"\u0306", u8"\u00b7"};

std::unordered_set<char> PTKH = {'p', 't', 'k', 'h'};

Tone getToneFromKeyMap(std::unordered_map<Tone, char> map, char ch) {
    for (const auto &it : map) {
        if (it.second == ch) {
            return it.first;
        }
    }

    return Tone::NaT;
}

Tone getToneFromDigit(char ch) { return getToneFromKeyMap(TONE_DIGIT_MAP, ch); }

Tone getToneFromTelex(char ch) { return getToneFromKeyMap(TONE_TELEX_MAP, ch); }

std::string stripDiacritics(const std::string &s) {
    std::string sRet = boost::locale::normalize(s, boost::locale::norm_nfd);
    for (std::string tone : TONES) {
        size_t found = sRet.find(tone);
        if (found != std::string::npos) {
            sRet.erase(found, tone.size());
            break;
        }
    }
    return sRet;
}

// s must be one syllable
std::string placeToneOnSyllable(const std::string &syllable,
                                const std::string &tone) {
    static boost::regex e(u8"o[ae][mnptkh]");
    static std::string ordered_vowel_matches[] = {u8"o", u8"a",  u8"e", u8"u",
                                                  u8"i", u8"ng", u8"m"};

    std::string sRet = stripDiacritics(syllable);

    BOOST_LOG_TRIVIAL(debug) << boost::format("stripped syl: %1%") % sRet;

    boost::smatch match;
    size_t found;

    if (regex_search(sRet, match, e)) {
        found = match.position() + 1;
    } else {
        for (std::string v : ordered_vowel_matches) {
            found = sRet.find(v);

            if (found != std::string::npos) {
                break;
            }
        }
    }

    if (found == std::string::npos) {
        return syllable;
    }

    sRet.insert(found + 1, tone);

    return sRet;
}

Buffer::Buffer()
    : syllables_(), cursor_(0, 0), segmentOffsets_(),
      toneKeys_(ToneKeys::Numeric) {}

bool Buffer::insert(char ch) {
    Syllable *curr = &syllables_[cursor_.first];

    Tone tone = Tone::NaT;

    if (toneKeys_ == ToneKeys::Numeric) {
        tone = getToneFromDigit(ch);
    } else if (toneKeys_ == ToneKeys::Telex) {
        tone = getToneFromTelex(ch);

        char end = curr->unicode.back();

        if ((PTKH.find(end) != PTKH.end()) && tone == Tone::T8) {
            tone = Tone::T7;
        } else if ((PTKH.find(end) == PTKH.end() && tone == Tone::T7)) {
            tone = Tone::T8;
        }
    }

    if (tone != Tone::NaT) {
        int prevLength = curr->display.size();

        curr->ascii.push_back(ch);
        curr->tone = tone;
        curr->unicode =
            placeToneOnSyllable(curr->unicode, TONE_UTF_MAP.at(curr->tone));
        curr->display =
            boost::locale::normalize(curr->unicode, boost::locale::norm_nfc);

        cursor_.second += curr->display.size() - prevLength;
    } else if (isalpha(ch)) {
        curr->ascii.push_back(ch);
        curr->unicode.push_back(ch);
        curr->display.push_back(ch);

        cursor_.second++;
    } else {
        return false;
    }

    return true;
}

bool Buffer::selectCandidate(hanlo_t candidate) { return false; }

bool Buffer::setToneKeys(ToneKeys toneKeys) {
    toneKeys_ = toneKeys;
    return true; // TODO
}

} // namespace TaiKey

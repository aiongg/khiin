
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utf8cpp/utf8.h>

#include "buffer.h"
#include "common.h"
#include "trie.h"

namespace TaiKey {

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

boost::regex toneableLetters(u8"[aeioumn]");

std::string asciiToUnicode(std::string ascii, Tone tone, bool khin) {
    return "";
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

// Modifies tone
void checkTone78Swap(const std::string &unicodeSyllable, Tone &tone) {
    if (unicodeSyllable.empty())
        return;

    char end = unicodeSyllable.back();

    if ((PTKH.find(end) != PTKH.end()) && tone == Tone::T8) {
        tone = Tone::T7;
    } else if ((PTKH.find(end) == PTKH.end() && tone == Tone::T7)) {
        tone = Tone::T8;
    }
}

Buffer::Buffer() : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric) {
    syllables_.reserve(20);
    syllables_.push_back(Syllable());
}

std::string Buffer::getDisplayBuffer() {
    std::string ret = "";
    for (auto &it : syllables_) {
        ret += it.unicode;
    }
    return boost::locale::normalize(ret, boost::locale::norm_nfc);
}

int Buffer::getCursor() {
    int ret = 0;

    for (int i = 0; i < syllables_.size(); i++) {
        if (cursor_.first == i) {
            ret += cursor_.second;
            return ret;
        }

        ret += syllables_[i].display.size();
    }
}

retval_t Buffer::insert(char ch) {
    Syllable *syl = &syllables_[cursor_.first];
    int *curs = &cursor_.second;

    // 1. Handle tones
    Tone tone = Tone::NaT;

    // 1a. Get the tone
    // Numeric tones enabled, even in Telex mode
    if (isdigit(ch)) {
        // We can use the following to disable numeric tones in Telex Mode
        // if (toneKeys_ == ToneKeys::Numeric) {
        tone = getToneFromDigit(ch);
    } else if (toneKeys_ == ToneKeys::Telex) {
        tone = getToneFromTelex(ch);
        checkTone78Swap(syl->unicode, tone);
    }

    // 1b. handle Khin
    if (tone == Tone::TK) {
        if (syl->khin == true) {
            return TK_OK;
        }

        syl->ascii.insert(*curs, &ch);
        syl->unicode.insert(0, TONE_UTF_MAP.at(Tone::TK));
        syl->display.insert(0, TONE_UTF_MAP.at(Tone::TK));
        syl->khin = true;
        curs++;
    } else if (tone != Tone::NaT) {
        int prevLength = syl->display.size();

        syl->ascii.insert(*curs, &ch);

        // vowel no tone
        // vowel + tone
        // no vowel

        if (!regex_search(syl->ascii, toneableLetters)) {
            // cut
            return TK_TODO;
        } else {
            syl->tone = tone;
            syl->unicode =
                placeToneOnSyllable(syl->unicode, TONE_UTF_MAP.at(syl->tone));
            syl->display =
                boost::locale::normalize(syl->unicode, boost::locale::norm_nfc);

            cursor_.second +=
                utf8::distance(syl->display.begin(), syl->display.end()) -
                prevLength;
        }

    } else if (isalpha(ch)) {
        syl->ascii.push_back(ch);
        syl->unicode.push_back(ch);
        syl->display.push_back(ch);

        cursor_.second++;
    } else {
        return TK_ERROR;
    }

    return TK_OK;
}

retval_t Buffer::remove(CursorDirection dir) { return TK_TODO; }

retval_t Buffer::moveCursor(CursorDirection dir) { return TK_TODO; }

bool Buffer::selectCandidate(hanlo_t candidate) { return false; }

bool Buffer::setToneKeys(ToneKeys toneKeys) {
    toneKeys_ = toneKeys;
    return true; // TODO
}

bool Buffer::isCursorAtEnd() {
    return (cursor_.first == syllables_.size() - 1 &&
            cursor_.second == syllables_[cursor_.first].unicode.size());
}

} // namespace TaiKey

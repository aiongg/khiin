#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>
#include <utf8cpp/utf8.h>

#include "buffer.h"
#include "common.h"
#include "tmp.h"
#include "trie.h"

namespace b = boost;
namespace bloc = boost::locale;
namespace balg = boost::algorithm;

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
    std::string sRet = bloc::normalize(s, bloc::norm_nfd);
    for (std::string tone : TONES) {
        size_t found = sRet.find(tone);
        if (found != std::string::npos) {
            sRet.erase(found, tone.size());
            break;
        }
    }
    return sRet;
}

b::regex toneableLetters(u8"[aeioumn]");

std::string placeToneOnSyllable(std::string syllable, Tone tone) {
    if (tone == Tone::NaT) {
        return syllable;
    }

    static b::regex e(u8"o[ae][mnptkh]");
    static std::string ordered_vowel_matches[] = {u8"o", u8"a",  u8"e", u8"u",
                                                  u8"i", u8"ng", u8"m"};

    std::string sRet = stripDiacritics(syllable);

    BOOST_LOG_TRIVIAL(debug) << b::format("stripped syl: %1%") % sRet;

    b::smatch match;
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

    sRet.insert(found + 1, TONE_UTF_MAP.at(tone));

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

// Syllable

retval_t Syllable::asciiToUnicode() {
    std::string tmp = ascii;

    balg::replace_first(tmp, u8"nn", U8_NN);
    balg::replace_first(tmp, u8"ou", "o" + U8_OU);
    balg::replace_first(tmp, u8"oo", "o" + U8_OU);

    if (khin) {
        tmp.insert(0, U8_TK);
    }

    if (tone != Tone::NaT) {
        tmp.pop_back();
        tmp = placeToneOnSyllable(tmp, tone);
    }

    unicode = tmp;

    return TK_OK;
}

retval_t Syllable::asciiToUnicodeAndDisplay() {
    asciiToUnicode();
    display = bloc::normalize(unicode, bloc::norm_nfc);
    return TK_OK;
}

int Syllable::displaySize() {
    return utf8::distance(display.begin(), display.end());
}

// Buffer

Buffer::Buffer()
    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric),
      sylTrie_(tmpGetSylTrieFromFile()) {
    syllables_.reserve(20);
    syllables_.push_back(Syllable());
}

std::string Buffer::getDisplayBuffer() {
    std::vector<std::string> syls;
    for (auto &it : syllables_) {
        syls.push_back(it.display);
    }
    return balg::join(syls, u8" ");
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
    // With isdigit(ch), numeric tones are enabled
    // even in Telex mode
    if (isdigit(ch) || toneKeys_ == ToneKeys::Numeric) {
        return insertNumeric_(ch);
    } else if (toneKeys_ == ToneKeys::Telex) {
        return insertTelex_(ch);
    }
}

retval_t Buffer::remove(CursorDirection dir) { return TK_TODO; }

retval_t Buffer::moveCursor(CursorDirection dir) { return TK_TODO; }

retval_t Buffer::clear() {
    syllables_.clear();
    syllables_.reserve(20);
    cursor_.first = 0;
    cursor_.second = 0;
    segmentOffsets_.clear();

    syllables_.push_back(Syllable());

    return TK_TODO;
}

bool Buffer::selectCandidate(hanlo_t candidate) { return false; }

bool Buffer::setToneKeys(ToneKeys toneKeys) {
    toneKeys_ = toneKeys;
    return true; // TODO
}

bool Buffer::isCursorAtEnd_() {
    return (cursor_.first == syllables_.size() - 1 &&
            cursor_.second == syllables_[cursor_.first].displaySize());
}

retval_t Buffer::insertNumeric_(char ch) {
    Syllable *syl = &syllables_[cursor_.first];
    int *curs = &cursor_.second;

    Tone tone = getToneFromDigit(ch);

    return TK_TODO;
}

retval_t Buffer::insertTelex_(char ch) {
    // TODO: handle telex double press
    char lk = lastKey_;
    lastKey_ = ch;

    Syllable *syl = &syllables_[cursor_.first];
    int *curs = &cursor_.second;
    bool atEnd = isCursorAtEnd_();

    Tone tone = getToneFromTelex(ch);
    checkTone78Swap(syl->unicode, tone);

    // need function to go between ascii and display cursors.

    if (lk == ch) {
        auto start = syl->display.begin();
        auto it = syl->display.end();

        if (!atEnd) {
            it = start + (*curs);
        }

        if (ch == 'u' && utf8::prior(it, start) == 0x0358) {
            if (atEnd) {

                appendNewSyllable_();
                syl = &syllables_[cursor_.first];
                (*curs) = 0;

            }

            BOOST_LOG_TRIVIAL(debug) << b::format("found ouu");
        }
    }


    // when is tone a tone?
    // 1. if we are at the end and there isn't already a tone
    // 2. if we aren't at the end

    bool canPlaceTone = regex_search(syl->ascii, toneableLetters);

    if (canPlaceTone && tone != Tone::NaT &&
        ((atEnd && syl->tone == Tone::NaT) || (!atEnd))) {
        if (tone == Tone::TK) {
            if (syl->khin == true) {
                // TODO: How to handle if already khin?
                return TK_TODO;
            }

            syl->khin = true;

            // TODO: how to handle ascii for khin?
            syl->asciiToUnicodeAndDisplay();
            (*curs)++;

            return TK_OK;
        }

        if (!atEnd) {
            syl->ascii.pop_back();
            // Definitely a tone? Use digit
            syl->ascii.push_back(TONE_DIGIT_MAP.at(tone));
        } else {
            syl->ascii.push_back(ch);
        }

        syl->tone = tone;

        int prevLength = syl->display.size();
        syl->asciiToUnicodeAndDisplay();
        cursor_.second +=
            utf8::distance(syl->display.begin(), syl->display.end()) -
            prevLength;

        return TK_OK;
    }

    if (atEnd) {
        // TODO: check Trie for invalid syllables
        if (syl->tone != Tone::NaT) {
            appendNewSyllable_();
            syl = &syllables_[cursor_.first];
            (*curs) = 0;
        }
        syl->ascii.push_back(ch);
        syl->asciiToUnicodeAndDisplay();
        (*curs)++;

        return TK_OK;
    } else {
        return TK_TODO;
    }

    return TK_ERROR;
}

void Buffer::appendNewSyllable_() {
    syllables_.push_back(Syllable());
    cursor_.first++;
    cursor_.second = 0;
}

} // namespace TaiKey

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
#include "lomaji.h"
#include "tmp.h"
#include "trie.h"

namespace TaiKey {

// Syllable

retval_t Syllable::asciiToUnicodeAndDisplay() {
    unicode = asciiToUtf8(ascii, tone, khin);
    display = unicode;

    return TK_OK;
}

size_t Syllable::displayUtf8Size() { return utf8Size(display); }

size_t Syllable::getAsciiCursor(size_t idx) {
    return getAsciiCursorFromUtf8(ascii, display, idx);
}

// Buffer

Buffer::Buffer()
    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_(),
      sylTrie_(tmpGetSylTrieFromFile()) {
    syllables_.reserve(20);
    syllables_.push_back(Syllable());
}

std::string Buffer::getDisplayBuffer() {
    std::vector<std::string> syls;
    for (auto &it : syllables_) {
        syls.push_back(it.display);
    }
    return boost::algorithm::join(syls, u8" ");
}

int Buffer::getCursor() {
    int ret = 0;

    for (int i = 0; i < syllables_.size(); i++) {
        if (cursor_.first == i) {
            ret += cursor_.second;
            return ret;
        }

        ret += syllables_[i].displayUtf8Size() + 1;
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

bool cursorSkipsCodepoint(uint32_t cp) {
    return (0x0300 <= cp && cp <= 0x0358);
}

retval_t Buffer::moveCursor(CursorDirection dir) {
    if (dir == CursorDirection::L) {
        if (cursor_.first == 0 && cursor_.second == 0) {
            return TK_OK;
        }

        if (cursor_.second == 0) {
            cursor_.first--;
            cursor_.second = syllables_[cursor_.first].displayUtf8Size();
            return TK_OK;
        }

        const std::string u = syllables_[cursor_.first].display;
        auto it = u.begin();
        utf8::advance(it, cursor_.second, u.end());
        auto cp = utf8::prior(it, u.begin());
        while (cursorSkipsCodepoint(cp)) {
            cp = utf8::prior(it, u.begin());
        }

        cursor_.second = utf8::distance(u.begin(), it);

        return TK_OK;
    }

    if (dir == CursorDirection::R) {
        if (isCursorAtEnd_()) {
            return TK_OK;
        }

        const std::string u = syllables_[cursor_.first].display;

        if (cursor_.second == utf8Size(u)) {
            cursor_.first++;
            cursor_.second = 0;

            return TK_OK;
        }

        auto it = u.begin();
        utf8::advance(it, cursor_.second, u.end());
        auto cp = utf8::next(it, u.end());
        while (cursorSkipsCodepoint(cp)) {
            cp = utf8::next(it, u.end());
        }
        cursor_.second = utf8::distance(u.begin(), it);

        return TK_OK;
    }

    return TK_ERROR;
}

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
            cursor_.second == syllables_[cursor_.first].displayUtf8Size());
}

retval_t Buffer::insertNumeric_(char ch) {
    Syllable *syl = &syllables_[cursor_.first];
    size_t *curs = &cursor_.second;

    Tone tone = getToneFromDigit(ch);

    syl->ascii.push_back(ch);
    syl->asciiToUnicodeAndDisplay();
    (*curs)++;

    return TK_TODO;
}

boost::regex toneableLetters(u8"[aeioumn]");

retval_t Buffer::insertTelex_(char ch) {
    // TODO: handle telex double press
    char lk = lastKey_;
    lastKey_ = ch;

    Syllable *syl = &syllables_[cursor_.first];
    size_t *curs = &cursor_.second;
    bool atEnd = isCursorAtEnd_();

    Tone tone = getToneFromTelex(ch);
    tone = checkTone78Swap(syl->unicode, tone);

    if (lk == ch) {
        auto start = syl->display.begin();
        auto it = syl->display.end();

        if (!atEnd) {
            it = start + (*curs);
        }

        if (ch == 'u' && utf8::prior(it, start) == 0x0358) {
            if (atEnd) {
                syl->ascii.pop_back();
                syl->asciiToUnicodeAndDisplay();
                appendNewSyllable_();
                syl = &syllables_[cursor_.first];
                (*curs) = 0;
            }
        } else if (tone != Tone::NaT) {
            std::string tmp = toNFD(syl->unicode);

            if (utf8::prior(tmp.end(), tmp.begin()) == TONE_U32_MAP.at(tone)) {
                syl->ascii.pop_back();
                syl->tone = Tone::NaT;
                syl->asciiToUnicodeAndDisplay();
                appendNewSyllable_();
                syl = &syllables_[cursor_.first];
                (*curs) = 0;
            }
        }
    }

    // when is tone a tone?
    // 1. if we are at the end and there isn't already a tone
    // 2. if we aren't at the end
    size_t acurs = syl->getAsciiCursor(*curs);
    bool canPlaceTone =
        regex_search(syl->ascii.substr(0, acurs), toneableLetters);

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

        int prevLength = syl->displayUtf8Size();
        syl->asciiToUnicodeAndDisplay();
        cursor_.second += syl->displayUtf8Size() - prevLength;

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
        syl->ascii.insert(*curs, 1, ch);
        syl->asciiToUnicodeAndDisplay();
        (*curs)++;

        return TK_OK;
    }

    return TK_ERROR;
}

void Buffer::appendNewSyllable_() {
    syllables_.push_back(Syllable());
    cursor_.first++;
    cursor_.second = 0;
}

} // namespace TaiKey

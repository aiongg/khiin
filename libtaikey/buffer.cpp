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
#include "trie.h"

namespace TaiKey {

// Syllable

auto Syllable::asciiToUnicodeAndDisplay() -> RetVal {
    unicode = asciiToUtf8(ascii, tone, khin);
    display = unicode;

    return RetVal::OK;
}

auto Syllable::displayUtf8Size() -> ptrdiff_t { return utf8Size(display); }

auto Syllable::getAsciiCursor(size_t idx) -> ptrdiff_t {
    return getAsciiCursorFromUtf8(ascii, display, idx);
}

// Buffer

Buffer::Buffer()
    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_(),
      dictTrie_(std::shared_ptr<Trie>(new Trie())),
      sylSplitter_(std::shared_ptr<Splitter>(new Splitter())) {
    clear();
}

Buffer::Buffer(std::shared_ptr<Trie> dictTrie,
               std::shared_ptr<Splitter> sylSplitter)
    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_() {
    clear();
    dictTrie_ = dictTrie;
    sylSplitter_ = sylSplitter;
}

auto Buffer::getDisplayBuffer() -> std::string {
    std::vector<std::string> syls;
    for (auto &it : syllables_) {
        syls.push_back(it.display);
    }
    return boost::algorithm::join(syls, u8" ");
}

auto Buffer::getCursor() -> size_t {
    auto ret = size_t(0);

    for (int i = 0; i < syllables_.size(); i++) {
        if (cursor_.first == i) {
            ret += cursor_.second;
            return ret;
        }

        ret += syllables_[i].displayUtf8Size() + 1;
    }

    return ret;
}

auto Buffer::insert(char ch) -> RetVal {
    // With isdigit(ch), numeric tones are enabled
    // even in Telex mode
    if (isdigit(ch) || toneKeys_ == ToneKeys::Numeric) {
        return insertNumeric_(ch);
    } else if (toneKeys_ == ToneKeys::Telex) {
        return insertTelex_(ch);
    }

    return RetVal::NotConsumed;
}

auto Buffer::remove(CursorDirection dir) -> RetVal { return RetVal::TODO; }

auto cursorSkipsCodepoint(uint32_t cp) {
    return (0x0300 <= cp && cp <= 0x0358);
}

auto Buffer::moveCursor(CursorDirection dir) -> RetVal {
    if (dir == CursorDirection::L) {
        if (cursor_.first == 0 && cursor_.second == 0) {
            return RetVal::OK;
        }

        if (cursor_.second == 0) {
            cursor_.first--;
            cursor_.second = syllables_[cursor_.first].displayUtf8Size();
            return RetVal::OK;
        }

        const std::string u = syllables_[cursor_.first].display;
        auto it = u.begin();
        utf8::advance(it, cursor_.second, u.end());
        auto cp = utf8::prior(it, u.begin());
        while (cursorSkipsCodepoint(cp)) {
            cp = utf8::prior(it, u.begin());
        }

        cursor_.second = utf8::distance(u.begin(), it);

        return RetVal::OK;
    }

    if (dir == CursorDirection::R) {
        if (isCursorAtEnd_()) {
            return RetVal::OK;
        }

        const std::string u = syllables_[cursor_.first].display;

        if (cursor_.second == utf8Size(u)) {
            cursor_.first++;
            cursor_.second = 0;

            return RetVal::OK;
        }

        auto it = u.begin();
        utf8::advance(it, cursor_.second, u.end());
        auto cp = utf8::next(it, u.end());
        while (cursorSkipsCodepoint(cp)) {
            cp = utf8::next(it, u.end());
        }
        cursor_.second = utf8::distance(u.begin(), it);

        return RetVal::OK;
    }

    return RetVal::Error;
}

auto Buffer::clear() -> RetVal {
    syllables_.clear();
    syllables_.reserve(20);
    cursor_.first = 0;
    cursor_.second = 0;
    segmentOffsets_.clear();

    syllables_.push_back(Syllable());

    return RetVal::TODO;
}

auto Buffer::selectCandidate(Hanlo candidate) { return false; }

auto Buffer::setToneKeys(ToneKeys toneKeys) -> RetVal {
    toneKeys_ = toneKeys;
    return RetVal::TODO;
}

auto Buffer::isCursorAtEnd_() -> bool {
    return (cursor_.first == syllables_.size() - 1 &&
            cursor_.second == syllables_[cursor_.first].displayUtf8Size());
}

auto Buffer::insertNumeric_(char ch) -> RetVal {
    Syllable *syl = &syllables_[cursor_.first];
    size_t *curs = &cursor_.second;

    Tone tone = getToneFromDigit(ch);

    syl->ascii.push_back(ch);
    syl->asciiToUnicodeAndDisplay();
    (*curs)++;

    return RetVal::TODO;
}

boost::regex toneableLetters(u8"[aeioumn]");

auto handleFuzzy(char ch) {
    // insert ch into the raw buffer
    // send rawbuffer to splitsyllables
    // get back VStr of syllables
    // display in buffer
    // send VStr to findMultiCandidate
    // -> 
    // 
    // send rawbuffer to findWordCandidates
}

auto Buffer::insertTelex_(char ch) -> RetVal {
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
                return RetVal::TODO;
            }

            syl->khin = true;

            // TODO: how to handle ascii for khin?
            syl->asciiToUnicodeAndDisplay();
            (*curs)++;

            return RetVal::OK;
        }

        if (!atEnd) {
            syl->ascii.pop_back();
            // Definitely a tone? Use digit
            syl->ascii.push_back(TONE_DIGIT_MAP.at(tone));
        } else {
            syl->ascii.push_back(ch);
        }

        syl->tone = tone;

        auto prevLength = syl->displayUtf8Size();
        syl->asciiToUnicodeAndDisplay();
        cursor_.second += syl->displayUtf8Size() - prevLength;

        return RetVal::OK;
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

        return RetVal::OK;
    } else {
        syl->ascii.insert(*curs, 1, ch);
        syl->asciiToUnicodeAndDisplay();
        (*curs)++;

        return RetVal::OK;
    }

    return RetVal::Error;
}

auto Buffer::appendNewSyllable_() -> void {
    syllables_.push_back(Syllable());
    cursor_.first++;
    cursor_.second = 0;
}

} // namespace TaiKey

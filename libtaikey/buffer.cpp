#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/regex.hpp>

#include <utf8cpp/utf8.h>

#include "buffer.h"
#include "common.h"
#include "lomaji.h"
#include "trie.h"

namespace TaiKey {

// Local utility methods

auto static asciiLettersPerCodepoint(uint32_t cp) {
    if (cp == ' ' || (0x0300 <= cp && cp <= 0x030d)) {
        return 0;
    } else if (cp == 0x207f || cp == 0x1e72 || cp == 0x1e73) {
        return 2;
    }

    return 1;
}

auto cursorSkipsCodepoint(uint32_t cp) { return 0x0300 <= cp && cp <= 0x0358; }

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

// Syllable

auto Syllable::asciiToUnicodeAndDisplay() -> RetVal {
    unicode = asciiSyllableToUtf8(ascii, tone, khin);
    display = unicode;

    return RetVal::OK;
}

auto Syllable::displayUtf8Size() -> ptrdiff_t { return utf8Size(display); }

auto Syllable::getAsciiCursor(size_t idx) -> ptrdiff_t {
    return getAsciiCursorFromUtf8(ascii, display, idx);
}

// Buffer

// Buffer::Buffer()
//    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_(),
//      dictTrie_(std::shared_ptr<Trie>(new Trie())),
//      sylSplitter_(std::shared_ptr<Splitter>(new Splitter())) {
//    clear();
//}
//
// Buffer::Buffer(std::shared_ptr<Trie> dictTrie,
//               std::shared_ptr<Splitter> sylSplitter)
//    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_() {
//    clear();
//    dictTrie_ = dictTrie;
//    sylSplitter_ = sylSplitter;
//}

Buffer::Buffer(CandidateFinder &candidateFinder)
    : candidateFinder_(candidateFinder) {}

// Public

auto Buffer::clear() -> RetVal {
    rawBuffer_ = "";
    rawCursor_ = 0;
    displayBuffer_ = "";
    displayCursor_ = 0;
    primaryCandidate_.clear();
    candidates_.clear();

    // syllables_.clear();
    // syllables_.reserve(20);
    // cursor_.first = 0;
    // cursor_.second = 0;
    // segmentOffsets_.clear();

    // syllables_.push_back(Syllable());

    return RetVal::TODO;
}

auto Buffer::getDisplayBuffer() -> std::string { return displayBuffer_; }

auto Buffer::getCursor() -> size_t {
    return displayCursor_;
    // auto ret = size_t(0);

    // for (int i = 0; i < syllables_.size(); i++) {
    //    if (cursor_.first == i) {
    //        ret += cursor_.second;
    //        return ret;
    //    }

    //    ret += syllables_[i].displayUtf8Size() + 1;
    //}

    // return ret;
}

auto Buffer::insert(char ch) -> RetVal {
    /*
     * ch must match: [A-Za-z0-9-]
     *
     * Cases:
     *   1. Normal / Pro input mode
     *   2. Numeric / Telex tones
     *   3. Fuzzy / Exact tones
     *   4. Lazy / Quick commit
     */

    switch (inputMode_) {
    case InputMode::Normal:
        return insertNormal_(ch);
    }

    // With isdigit(ch), numeric tones are enabled
    // even in Telex mode
    // if (isdigit(ch) || toneKeys_ == ToneKeys::Numeric) {
    //    return insertNumeric_(ch);
    //} else if (toneKeys_ == ToneKeys::Telex) {
    //    return insertTelex_(ch);
    //}

    return RetVal::NotConsumed;
}

auto Buffer::moveCursor(CursorDirection dir) -> RetVal {
    if (dir == CursorDirection::L) {
        auto moveAscii = 0;

        if (displayCursor_ != 0) {
            auto it = displayBuffer_.begin();
            utf8::advance(it, displayCursor_, displayBuffer_.end());
            auto cp = utf8::prior(it, displayBuffer_.begin());
            moveAscii += asciiLettersPerCodepoint(cp);

            while (cursorSkipsCodepoint(cp)) {
                cp = utf8::prior(it, displayBuffer_.begin());
                moveAscii += asciiLettersPerCodepoint(cp);
            }

            displayCursor_ = utf8::distance(displayBuffer_.begin(), it);
            rawCursor_ -= moveAscii;
        }

        return RetVal::OK;
    }

    if (dir == CursorDirection::R) {
        auto moveAscii = 0;

        if (!isCursorAtEnd_()) {
            auto it = displayBuffer_.begin();
            utf8::advance(it, displayCursor_, displayBuffer_.end());
            auto cp = utf8::next(it, displayBuffer_.end());
            moveAscii += asciiLettersPerCodepoint(cp);
            while (cursorSkipsCodepoint(cp)) {
                cp = utf8::next(it, displayBuffer_.end());
                moveAscii += asciiLettersPerCodepoint(cp);
            }

            displayCursor_ = utf8::distance(displayBuffer_.begin(), it);
            rawCursor_ += moveAscii;
        }

        return RetVal::OK;
    }

    return RetVal::Error;
}

auto Buffer::remove(CursorDirection dir) -> RetVal { return RetVal::TODO; }

auto Buffer::selectCandidate(Hanlo candidate) { return false; }

auto Buffer::setToneKeys(ToneKeys toneKeys) -> RetVal {
    toneKeys_ = toneKeys;
    return RetVal::TODO;
}

// Private

auto Buffer::appendNewSyllable_() -> void {
    syllables_.push_back(Syllable());
    cursor_.first++;
    cursor_.second = 0;
}

auto Buffer::findCandidateBegin() -> std::pair<size_t, Utf8Size> {
    return std::pair<size_t, Utf8Size>();
}

auto Buffer::findSyllableBegin() -> std::pair<size_t, Utf8Size> {
    return std::pair<size_t, Utf8Size>();
}

auto Buffer::isCursorAtEnd_() -> bool {
    return displayCursor_ == utf8Size(displayBuffer_);
    // return (cursor_.first == syllables_.size() - 1 &&
    //        cursor_.second == syllables_[cursor_.first].displayUtf8Size());
}

auto Buffer::insertNormal_(char ch) -> RetVal {
    /**
     * Cases:
     *   1. Letter at the end: insert letter, update candidates up to 4
     * syllables prior
     *   2. Tone at the end: if there's already a tone in, change the tone;
     * otherwise, as (1)
     *   3. Letter in the middle: insert letter, update candidates from
     * beginning of current syllable
     *   4. Tone in middle: change tone on current syllable
     */
    auto rawBufferCutoff = size_t(0);
    auto primaryCandidateCutoff = size_t(0);

    auto prevRawBufLen = rawBuffer_.size();
    rawBuffer_.insert(rawCursor_, 1, ch);
    rawCursor_ += 1;

    if (isCursorAtEnd_()) {
        if (primaryCandidate_.size() > 4) {
            auto consumed = size_t(0);
            auto idx = 0;
            auto it = primaryCandidate_.begin();

            while (it != primaryCandidate_.end() - 4) {
                consumed += (*it).ascii.size();
                it++;
            }

            while (it != primaryCandidate_.end()) {
                it = primaryCandidate_.erase(it);
            }

            auto nextCandidate = candidateFinder_.findPrimaryCandidate(
                std::string(rawBuffer_.begin() + consumed, rawBuffer_.end()),
                toneMode_ == ToneMode::Fuzzy, primaryCandidate_.back());

            primaryCandidate_.insert(primaryCandidate_.end(),
                                     nextCandidate.cbegin(),
                                     nextCandidate.cend());
        } else {
            primaryCandidate_ = candidateFinder_.findPrimaryCandidate(
                rawBuffer_, toneMode_ == ToneMode::Fuzzy, Candidate());

            candidates_ = candidateFinder_.findCandidates(
                rawBuffer_, toneMode_ == ToneMode::Fuzzy, Candidate());
        }
    } else {
    }

    auto prevBufLen = utf8Size(displayBuffer_);
    updateDisplayBuffer_();
    displayCursor_ += (utf8Size(displayBuffer_) - prevBufLen);

    return RetVal::TODO;
}

auto Buffer::insertNumeric_(char ch) -> RetVal {
    rawBuffer_.push_back(ch);

    primaryCandidate_ = candidateFinder_.findPrimaryCandidate(
        rawBuffer_, toneMode_ == ToneMode::Fuzzy, "");
    candidates_ = candidateFinder_.findCandidates(
        rawBuffer_, toneMode_ == ToneMode::Fuzzy, "");

    Syllable *syl = &syllables_[cursor_.first];
    size_t *curs = &cursor_.second;

    Tone tone = getToneFromDigit(ch);

    syl->ascii.push_back(ch);
    syl->asciiToUnicodeAndDisplay();
    (*curs)++;

    return RetVal::TODO;
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

            if (utf8::prior(tmp.end(), tmp.begin()) ==
                ToneToUint32Map.at(tone)) {
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
        boost::regex_search(syl->ascii.substr(0, acurs), toneableLetters);

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
            syl->ascii.push_back(ToneToDigitMap.at(tone));
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

auto isOnlyHyphens(std::string s) {
    for (const auto &c : s) {
        if (c != '-') {
            return false;
        }
    }

    return true;
}

auto Buffer::updateDisplayBuffer_() -> void {
    // editing/normal/fuzzy:
    if (primaryCandidate_.empty()) {
        return;
    }

    auto rawSyllableOffsets = std::vector<size_t>();
    auto displayCandidateOffsets = std::vector<Utf8Size>();
    auto displaySyllableOffsets = std::vector<Utf8Size>();

    auto rawOffset = size_t(0);
    auto displayOffset = Utf8Size(0);
    auto displayBuffer = std::string();

    for (const auto &c : primaryCandidate_) {
        if (isOnlyHyphens(c.ascii)) {
            if (displayBuffer.back() == ' ') {
                displayBuffer.pop_back();
            }
            displayBuffer += c.ascii;
            displayOffset += c.ascii.size() - 1;
            continue;
        }

        auto spacedSyls = spaceAsciiByUtf8(c.ascii, c.input);

        for (const auto &s : spacedSyls | boost::adaptors::indexed()) {
            if (isOnlyHyphens(s.value())) {
                if (displayBuffer.back() == ' ') {
                    displayBuffer.pop_back();
                }
                displayBuffer += s.value();
                displayOffset += s.value().size() - 1;
                rawOffset += s.value().size();
                continue;
            }

            rawSyllableOffsets.push_back(rawOffset);
            rawOffset += s.value().size();

            auto displaySyl = asciiSyllableToUtf8(s.value());
            displayBuffer += displaySyl + " ";

            displaySyllableOffsets.push_back(displayOffset);
            if (s.index() == 0) {
                displayCandidateOffsets.push_back(displayOffset);
            }
            displayOffset += utf8Size(displaySyl) + 1;
        }
    }

    displayBuffer.pop_back();

    if (rawBuffer_.back() == '-') {
        displayBuffer.push_back('-');
    }

    rawBufferSyllableOffsets_ = std::move(rawSyllableOffsets);
    displayBufferSyllableOffsets_ = std::move(displaySyllableOffsets);
    displayBufferCandidateOffsets_ = std::move(displayCandidateOffsets);
    displayBuffer_ = std::move(displayBuffer);

    BOOST_LOG_TRIVIAL(debug) << "IN:  " << displayBuffer_;
    auto tmp = std::string();
    for (auto &pc : primaryCandidate_) {
        tmp += pc.output;
    }
    BOOST_LOG_TRIVIAL(debug) << "OUT: " << tmp;
}

} // namespace TaiKey

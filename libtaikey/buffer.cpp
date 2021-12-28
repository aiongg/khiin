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

// BufferManager

// BufferManager::BufferManager()
//    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_(),
//      dictTrie_(std::shared_ptr<Trie>(new Trie())),
//      sylSplitter_(std::shared_ptr<Splitter>(new Splitter())) {
//    clear();
//}
//
// BufferManager::BufferManager(std::shared_ptr<Trie> dictTrie,
//               std::shared_ptr<Splitter> sylSplitter)
//    : syllables_(), cursor_(0, 0), toneKeys_(ToneKeys::Numeric), lastKey_() {
//    clear();
//    dictTrie_ = dictTrie;
//    sylSplitter_ = sylSplitter;
//}

BufferManager::BufferManager(CandidateFinder &candidateFinder)
    : candidateFinder_(candidateFinder) {}

// Public

auto BufferManager::clear() -> RetVal {
    rawCursor_ = 0;
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

auto BufferManager::getDisplayBuffer() -> std::string { return dispBuf_.text; }

auto BufferManager::getCursor() -> size_t {
    return dispBuf_.cursor;
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

auto BufferManager::insert(char ch) -> RetVal {
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

auto BufferManager::moveCursor(CursorDirection dir) -> RetVal {
    if (dir == CursorDirection::L) {
        auto moveAscii = 0;

        if (dispBuf_.cursor != 0) {
            auto it = dispBuf_.text.begin();
            utf8::advance(it, dispBuf_.cursor, dispBuf_.text.end());
            auto cp = utf8::prior(it, dispBuf_.text.begin());
            moveAscii += asciiLettersPerCodepoint(cp);

            while (cursorSkipsCodepoint(cp)) {
                cp = utf8::prior(it, dispBuf_.text.begin());
                moveAscii += asciiLettersPerCodepoint(cp);
            }

            dispBuf_.cursor = utf8::distance(dispBuf_.text.begin(), it);
            rawBuf_.cursor -= moveAscii;
        }

        return RetVal::OK;
    }

    if (dir == CursorDirection::R) {
        auto moveAscii = 0;

        if (!isCursorAtEnd_()) {
            auto it = dispBuf_.text.begin();
            utf8::advance(it, dispBuf_.cursor, dispBuf_.text.end());
            auto cp = utf8::next(it, dispBuf_.text.end());
            moveAscii += asciiLettersPerCodepoint(cp);
            while (cursorSkipsCodepoint(cp)) {
                cp = utf8::next(it, dispBuf_.text.end());
                moveAscii += asciiLettersPerCodepoint(cp);
            }

            dispBuf_.cursor = utf8::distance(dispBuf_.text.begin(), it);
            rawCursor_ += moveAscii;
        }

        return RetVal::OK;
    }

    return RetVal::Error;
}

auto BufferManager::remove(CursorDirection dir) -> RetVal {
    if (dir == CursorDirection::L && rawBuf_.cursor > 0) {
        rawBuf_.text.erase(rawBuf_.cursor - 1, 1);
        rawBuf_.cursor--;
    }

    if (dir == CursorDirection::R && rawBuf_.cursor < rawBuf_.text.size()) {
        rawBuf_.text.erase(rawBuf_.cursor, 1);
    }

    // chiahpa

    getFuzzyCandidates_();

    return RetVal::TODO;
}

auto BufferManager::selectCandidate(Hanlo candidate) { return false; }

auto BufferManager::setToneKeys(ToneKeys toneKeys) -> RetVal {
    toneKeys_ = toneKeys;
    return RetVal::TODO;
}

// Private

auto BufferManager::appendNewSyllable_() -> void {
    syllables_.push_back(Syllable());
    cursor_.first++;
    cursor_.second = 0;
}

auto BufferManager::findCandidateAtCursor() -> size_t {
    if (rawBuf_.candOffsets.size() == 0) {
        return 0;
    }

    auto &dbco = rawBuf_.candOffsets;
    auto dispIt = std::upper_bound(dbco.cbegin(), dbco.cend(), rawBuf_.cursor);

    if (dispIt == dbco.cend()) {
        return rawBuf_.candOffsets.size() - 1;
    }

    return static_cast<size_t>(std::distance(dbco.cbegin(), dispIt));
}

auto BufferManager::findSyllableBegin() -> std::pair<size_t, Utf8Size> {
    return std::pair<size_t, Utf8Size>();
}

auto BufferManager::getFuzzyCandidates_() -> void {
    auto candPos = findCandidateAtCursor();
    auto startCand = size_t(0);
    auto rawStart = size_t(0);

    if (candPos > 4) {
        startCand = candPos - 4;
        rawStart = rawBuf_.candOffsets[startCand];
    }

    auto consumed = size_t(0);
    auto idx = 0;
    auto it = primaryCandidate_.begin() + startCand;

    while (it != primaryCandidate_.end()) {
        it = primaryCandidate_.erase(it);
    }

    Candidate lgram;

    if (primaryCandidate_.size() > 0) {
        lgram = primaryCandidate_[startCand - 1];
    } else {
        lgram = Candidate();
    }

    auto searchStr =
        std::string(rawBuf_.text.begin() + rawStart, rawBuf_.text.end());

    auto nextCandidate = candidateFinder_.findPrimaryCandidate(
        searchStr, toneMode_ == ToneMode::Fuzzy, lgram);

    primaryCandidate_.insert(primaryCandidate_.end(), nextCandidate.cbegin(),
                             nextCandidate.cend());

    candidates_ = candidateFinder_.findCandidates(
        searchStr, toneMode_ == ToneMode::Fuzzy, Candidate());

    auto prevBufLen = utf8Size(dispBuf_.text);
    auto spaceBeforeCursor = false;
    if (dispBuf_.cursor > 0 && dispBuf_.text[dispBuf_.cursor - 1] == ' ') {
        spaceBeforeCursor = true;
    }
    updateDisplayBuffer_();
    auto newBufLen = utf8Size(dispBuf_.text);

    if (spaceBeforeCursor && newBufLen < prevBufLen) {
        dispBuf_.cursor += (newBufLen - prevBufLen + 1);
    } else {
        dispBuf_.cursor += (newBufLen - prevBufLen);
    }
}

auto BufferManager::isCursorAtEnd_() -> bool {
    return dispBuf_.cursor == utf8Size(dispBuf_.text);
}

auto BufferManager::insertNormal_(char ch) -> RetVal {
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
    auto prevRawBufLen = rawBuf_.text.size();
    rawBuf_.text.insert(rawBuf_.cursor, 1, ch);
    rawBuf_.cursor += 1;

    getFuzzyCandidates_();

    return RetVal::TODO;
}

auto BufferManager::insertNumeric_(char ch) -> RetVal {
    rawBuf_.text.push_back(ch);

    primaryCandidate_ = candidateFinder_.findPrimaryCandidate(
        rawBuf_.text, toneMode_ == ToneMode::Fuzzy, "");
    candidates_ = candidateFinder_.findCandidates(
        rawBuf_.text, toneMode_ == ToneMode::Fuzzy, "");

    Syllable *syl = &syllables_[cursor_.first];
    size_t *curs = &cursor_.second;

    Tone tone = getToneFromDigit(ch);

    syl->ascii.push_back(ch);
    syl->asciiToUnicodeAndDisplay();
    (*curs)++;

    return RetVal::TODO;
}

auto BufferManager::insertTelex_(char ch) -> RetVal {
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

auto BufferManager::updateDisplayBuffer_() -> void {
    // editing/normal/fuzzy:
    if (primaryCandidate_.empty()) {
        return;
    }

    auto rawSylOffsets = std::vector<size_t>();
    auto rawCandOffsets = std::vector<size_t>();
    auto dbSylOffsets = std::vector<Utf8Size>();
    auto dbCandOffsets = std::vector<Utf8Size>();

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

            rawSylOffsets.push_back(rawOffset);

            auto displaySyl = asciiSyllableToUtf8(s.value());
            displayBuffer += displaySyl + " ";

            dbSylOffsets.push_back(displayOffset);
            if (s.index() == 0) {
                rawCandOffsets.push_back(rawOffset);
                dbCandOffsets.push_back(displayOffset);
            }
            rawOffset += s.value().size();
            displayOffset += utf8Size(displaySyl) + 1;
        }
    }

    displayBuffer.pop_back();

    if (rawBuf_.text.back() == '-') {
        displayBuffer.push_back('-');
    }

    rawBuf_.sylOffsets = std::move(rawSylOffsets);
    rawBuf_.candOffsets = std::move(rawCandOffsets);
    dispBuf_.sylOffsets = std::move(dbSylOffsets);
    dispBuf_.candOffsets = std::move(dbCandOffsets);
    dispBuf_.text = std::move(displayBuffer);

    BOOST_LOG_TRIVIAL(debug) << "IN:  " << dispBuf_.text;
    auto tmp = std::string();
    for (auto &pc : primaryCandidate_) {
        tmp += pc.output;
    }
    BOOST_LOG_TRIVIAL(debug) << "OUT: " << tmp;
}

} // namespace TaiKey

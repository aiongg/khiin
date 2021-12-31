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

boost::regex toneableLetters(u8"[aeioumn]");

auto isOnlyHyphens(std::string s) {
    for (const auto &c : s) {
        if (c != '-') {
            return false;
        }
    }

    return true;
}

// Buffer

Buffer::Buffer()
    : text(""), cursor(0), sylOffsets(std::vector<size_t>()),
      candOffsets(std::vector<size_t>()) {}

auto Buffer::cursorIt() -> std::string::iterator {
    auto it = text.begin();
    utf8::advance(it, cursor, text.end());
    return it;
}

auto Buffer::ccursorIt() -> std::string::const_iterator {
    auto it = text.cbegin();
    utf8::advance(it, cursor, text.cend());
    return it;
}

auto Buffer::candBegin() -> std::string::iterator {
    if (candOffsets.size() < 2) {
        return text.begin();
    }

    auto it =
        std::upper_bound(candOffsets.cbegin(), candOffsets.cend(), cursor) - 1;

    auto ret = text.begin();
    utf8::advance(ret, *it, text.end());
    return ret;
}

auto Buffer::sylBegin() -> std::string::iterator {
    if (sylOffsets.size() < 2) {
        return text.begin();
    }

    auto it =
        std::upper_bound(sylOffsets.cbegin(), sylOffsets.cend(), cursor) - 1;

    auto ret = text.begin();
    utf8::advance(ret, *it, text.end());
    return ret;
}

auto Buffer::sylEnd() -> std::string::iterator {
    if (sylOffsets.size() < 2) {
        return text.end();
    }

    auto it = std::upper_bound(sylOffsets.cbegin(), sylOffsets.cend(), cursor);

    if (it == sylOffsets.cend()) {
        return text.end();
    }

    auto ret = text.begin();
    utf8::advance(ret, *it, text.end());
    return ret;
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

BufferManager::BufferManager(CandidateFinder &candidateFinder)
    : candidateFinder_(candidateFinder) {}

// Public

auto BufferManager::clear() -> RetVal {
    rawBuf_ = Buffer();
    dispBuf_ = Buffer();
    primaryCandidate_.clear();
    candidates_.clear();

    return RetVal::TODO;
}

auto BufferManager::getDisplayBuffer() -> std::string { return dispBuf_.text; }

auto BufferManager::getCursor() -> size_t { return dispBuf_.cursor; }

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

    return RetVal::NotConsumed;
}

auto BufferManager::moveCursor(CursorDirection dir) -> RetVal {
    if (dir == CursorDirection::L) {
        if (rawBuf_.cursor == 0) {
            return RetVal::OK;
        }

        auto r_it = rawBuf_.cursorIt();
        auto d_it = dispBuf_.cursorIt();

        parallelPrior(r_it, rawBuf_.text.begin(), d_it, dispBuf_.text.begin());
        rawBuf_.cursor = std::distance(rawBuf_.text.begin(), r_it);
        dispBuf_.cursor = utf8::distance(dispBuf_.text.begin(), d_it);

        return RetVal::OK;
    }

    if (dir == CursorDirection::R) {
        if (isCursorAtEnd_()) {
            return RetVal::OK;
        }

        auto r_it = rawBuf_.cursorIt();
        auto d_it = dispBuf_.cursorIt();

        parallelNext(r_it, rawBuf_.text.end(), d_it, dispBuf_.text.end());
        rawBuf_.cursor = std::distance(rawBuf_.text.begin(), r_it);
        dispBuf_.cursor = utf8::distance(dispBuf_.text.begin(), d_it);

        return RetVal::OK;
    }

    return RetVal::Error;
}

auto BufferManager::remove(CursorDirection dir) -> RetVal {
    auto extraSpaceInDisplay = false;
    if (auto it = dispBuf_.ccursorIt();
        it != dispBuf_.text.cend() && *it == ' ') {
        extraSpaceInDisplay = true;
    }

    if (dir == CursorDirection::L && rawBuf_.cursor > 0) {
        if (atSyllableStart_() && *(dispBuf_.sylBegin() - 1) == ' ') {
            return moveCursor(dir);
        } else {
            auto r_it = rawBuf_.cursorIt();
            auto d_it = dispBuf_.cursorIt();

            parallelPrior(r_it, rawBuf_.text.begin(), d_it,
                          dispBuf_.text.begin());

            if (hasToneDiacritic(std::string(d_it, dispBuf_.cursorIt()))) {
                removeToneFromRawBuffer_();
            }

            rawBuf_.text.erase(r_it, rawBuf_.cursorIt());
            rawBuf_.cursor = std::distance(rawBuf_.text.begin(), r_it);
        }
    }

    if (dir == CursorDirection::R && rawBuf_.cursor < rawBuf_.text.size()) {
        if (*(dispBuf_.cursorIt()) == ' ') {
            return moveCursor(dir);
        } else {
            auto r_it = rawBuf_.cursorIt();
            auto d_it = dispBuf_.cursorIt();

            parallelNext(r_it, rawBuf_.text.end(), d_it, dispBuf_.text.end());

            if (hasToneDiacritic(std::string(dispBuf_.cursorIt(), d_it))) {
                removeToneFromRawBuffer_();
            }

            rawBuf_.text.erase(rawBuf_.cursorIt(), r_it);
        }
    }

    getFuzzyCandidates_(findCandidateAtCursor_());
    updateDisplayBuffer_();

    if (dir == CursorDirection::L && *(dispBuf_.ccursorIt() - 1) == ' ' &&
        extraSpaceInDisplay) {
        dispBuf_.cursor--;
    }

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

auto BufferManager::atSyllableStart_() -> bool {
    return std::binary_search(dispBuf_.sylOffsets.begin(),
                              dispBuf_.sylOffsets.end(), dispBuf_.cursor);
}

auto BufferManager::findCandidateAtCursor_() -> size_t {
    if (rawBuf_.candOffsets.size() == 0) {
        return 0;
    }

    auto &offsets = rawBuf_.candOffsets;
    auto it =
        std::upper_bound(offsets.cbegin(), offsets.cend(), rawBuf_.cursor) - 1;

    if (it == offsets.cend()) {
        return offsets.size() - 1;
    }

    return static_cast<size_t>(std::distance(offsets.cbegin(), it));
}

auto BufferManager::getDispBufAt_(size_t index) -> uint32_t {
    auto it = dispBuf_.text.cbegin();
    utf8::advance(it, index, dispBuf_.text.cend());
    return utf8::peek_next(it, dispBuf_.text.cend());
}

auto BufferManager::getFuzzyCandidates_() -> void {
    auto candPos = findCandidateAtCursor_();
    auto startCand = candPos > 4 ? candPos - 4 : 0;

    getFuzzyCandidates_(startCand);
}

auto BufferManager::getFuzzyCandidates_(size_t startCand) -> void {
    auto rawStart = startCand > 0 ? rawBuf_.candOffsets[startCand] : 0;
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
}

auto BufferManager::isCursorAtEnd_() -> bool {
    return rawBuf_.cursor == rawBuf_.text.size();
}

auto BufferManager::insertNormal_(char ch) -> RetVal {
    /**
     * Cases:
     *   1. Letter at the end: insert letter, update candidates up to 4
     *      syllables prior
     *   2. Tone at the end: if there's already a tone in, change the tone;
     *      otherwise, as (1)
     *   3. Letter in the middle: insert letter, update candidates from
     *      beginning of current syllable
     *   4. Tone in middle: change tone on current syllable
     */
    auto prevRawBufLen = rawBuf_.text.size();
    rawBuf_.text.insert(rawBuf_.cursor, 1, ch);
    rawBuf_.cursor += 1;

    auto d_begin = dispBuf_.text.cbegin();
    auto d_it = dispBuf_.ccursorIt();
    auto stayLeft = true;
    if (d_it != d_begin) {
        if (utf8::prior(d_it, dispBuf_.text.cbegin()) == ' ') {
            stayLeft = false;
        }
    }

    if (isCursorAtEnd_()) {
        getFuzzyCandidates_();
    } else {
        getFuzzyCandidates_(findCandidateAtCursor_());
    }

    updateDisplayBuffer_();

    d_begin = dispBuf_.text.cbegin();
    d_it = dispBuf_.ccursorIt();
    if (d_it != d_begin) {
        if (stayLeft && utf8::prior(d_it, dispBuf_.text.cbegin()) == ' ') {
            --dispBuf_.cursor;
        }
    }

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

auto BufferManager::removeToneFromRawBuffer_() -> void {
    auto sylBegin = rawBuf_.sylBegin();
    auto sylIt = rawBuf_.sylEnd();
    --sylIt;

    while (sylIt != sylBegin) {
        if (isdigit(*sylIt) && (*sylIt) != '0') {
            if (rawBuf_.cursorIt() - 1 == sylIt) {
                rawBuf_.cursor--;
            }

            rawBuf_.text.erase(sylIt, sylIt + 1);
            break;
        }
        sylIt--;
    }
}

auto BufferManager::updateDisplayBuffer_() -> void {
    // editing/normal/fuzzy:
    if (primaryCandidate_.empty()) {
        return;
    }

    auto currBufSize = utf8Size(dispBuf_.text);

    auto rawSylOffsets = std::vector<size_t>();
    auto rawCandOffsets = std::vector<size_t>();
    auto dbSylOffsets = std::vector<Utf8Size>();
    auto dbCandOffsets = std::vector<Utf8Size>();

    auto rawOffset = size_t(0);
    auto displayOffset = Utf8Size(0);
    auto displayBuffer = std::string();

    auto khinNextCand = false;

    for (const auto &c : primaryCandidate_) {
        if (c.ascii == "--") {
            khinNextCand = true;
            continue;
        } else if (isOnlyHyphens(c.ascii)) {
            if (displayBuffer.back() == ' ') {
                displayBuffer.pop_back();
            }
            displayBuffer += c.ascii;
            displayOffset += c.ascii.size() - 1;
            continue;
        }

        auto spacedSyls = spaceAsciiByUtf8(c.ascii, c.input);

        for (const auto &s : spacedSyls | boost::adaptors::indexed()) {
            if (khinNextCand) {
                displayBuffer += "·";
            } else if (isOnlyHyphens(s.value())) {
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
            if (khinNextCand) {
                rawOffset += 2;
                displayOffset += 1;
                khinNextCand = false;
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

    updateDisplayCursor_();

    BOOST_LOG_TRIVIAL(debug) << "IN:  " << dispBuf_.text;
    auto tmp = std::string();
    for (auto &pc : primaryCandidate_) {
        tmp += pc.output;
    }
    BOOST_LOG_TRIVIAL(debug) << "OUT: " << tmp;
}

auto BufferManager::updateDisplayCursor_() -> void {
    if (isCursorAtEnd_()) {
        dispBuf_.cursor =
            utf8::distance(dispBuf_.text.begin(), dispBuf_.text.end());
    } else {
        auto r_target = rawBuf_.cursorIt();
        auto r_it = rawBuf_.candBegin();
        auto r_end = rawBuf_.text.end();

        auto currCand = findCandidateAtCursor_();
        auto d_it = dispBuf_.text.begin();
        utf8::advance(d_it, dispBuf_.candOffsets[findCandidateAtCursor_()],
                      dispBuf_.text.end());
        auto d_end = dispBuf_.text.end();

        while (r_it != r_end && r_it != r_target && d_it != d_end) {
            parallelNext(r_it, r_end, d_it, d_end);
        }

        dispBuf_.cursor = utf8::distance(dispBuf_.text.begin(), d_it);
    }
}

} // namespace TaiKey

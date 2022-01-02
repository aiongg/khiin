#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/regex.hpp>

#include <utf8cpp/utf8.h>

#include "buffer_manager.h"
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

// BufferManager

BufferManager::BufferManager(CandidateFinder &candidateFinder)
    : candidateFinder(candidateFinder) {}

// Public

auto BufferManager::clear() -> RetVal {
    rawBuf = Buffer();
    dispBuf = Buffer();
    primaryCandidate.clear();
    candidates.clear();

    return RetVal::TODO;
}

auto BufferManager::getDisplayBuffer() -> std::string { return dispBuf.text; }

auto BufferManager::getCursor() -> size_t { return dispBuf.cursor; }

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
        if (rawBuf.cursor == 0) {
            return RetVal::OK;
        }

        auto r_it = rawBuf.cursorIt();
        auto d_it = dispBuf.cursorIt();

        parallelPrior(r_it, rawBuf.text.begin(), d_it, dispBuf.text.begin());
        rawBuf.cursor = std::distance(rawBuf.text.begin(), r_it);
        dispBuf.cursor = utf8::distance(dispBuf.text.begin(), d_it);

        return RetVal::OK;
    }

    if (dir == CursorDirection::R) {
        if (isCursorAtEnd()) {
            return RetVal::OK;
        }

        auto r_it = rawBuf.cursorIt();
        auto d_it = dispBuf.cursorIt();

        parallelNext(r_it, rawBuf.text.end(), d_it, dispBuf.text.end());
        rawBuf.cursor = std::distance(rawBuf.text.begin(), r_it);
        dispBuf.cursor = utf8::distance(dispBuf.text.begin(), d_it);

        return RetVal::OK;
    }

    return RetVal::Error;
}

auto BufferManager::remove(CursorDirection dir) -> RetVal {
    auto extraSpaceInDisplay = false;
    if (auto it = dispBuf.ccursorIt();
        it != dispBuf.text.cend() && *it == ' ') {
        extraSpaceInDisplay = true;
    }

    if (dir == CursorDirection::L && rawBuf.cursor > 0) {
        if (atSyllableStart() && *(dispBuf.sylBegin() - 1) == ' ') {
            return moveCursor(dir);
        } else {
            auto r_it = rawBuf.cursorIt();
            auto d_it = dispBuf.cursorIt();

            parallelPrior(r_it, rawBuf.text.begin(), d_it,
                          dispBuf.text.begin());

            if (hasToneDiacritic(std::string(d_it, dispBuf.cursorIt()))) {
                removeToneFromRawBuffer();
            }

            rawBuf.text.erase(r_it, rawBuf.cursorIt());
            rawBuf.cursor = std::distance(rawBuf.text.begin(), r_it);
        }
    }

    if (dir == CursorDirection::R && rawBuf.cursor < rawBuf.text.size()) {
        if (*(dispBuf.cursorIt()) == ' ') {
            return moveCursor(dir);
        } else {
            auto r_it = rawBuf.cursorIt();
            auto d_it = dispBuf.cursorIt();

            parallelNext(r_it, rawBuf.text.end(), d_it, dispBuf.text.end());

            if (hasToneDiacritic(std::string(dispBuf.cursorIt(), d_it))) {
                removeToneFromRawBuffer();
            }

            rawBuf.text.erase(rawBuf.cursorIt(), r_it);
        }
    }

    getFuzzyCandidates(findCandidateAtCursor());
    updateDisplayBuffer();

    if (dir == CursorDirection::L && *(dispBuf.ccursorIt() - 1) == ' ' &&
        extraSpaceInDisplay) {
        dispBuf.cursor--;
    }

    return RetVal::TODO;
}

auto BufferManager::selectCandidate(Hanlo candidate) { return false; }

auto BufferManager::setToneKeys(ToneKeys toneKeys) -> RetVal {
    toneKeys_ = toneKeys;
    return RetVal::TODO;
}

auto BufferManager::spacebar() -> RetVal {
    if (empty()) {
        return RetVal::NotConsumed;
    }

    for (auto &c : primaryCandidate) {
        
    }

    return RetVal::TODO;
}

// Private

auto BufferManager::atSyllableStart() -> bool {
    return std::binary_search(dispBuf.sylOffsets.begin(),
                              dispBuf.sylOffsets.end(), dispBuf.cursor);
}

auto BufferManager::findCandidateAtCursor() -> size_t {
    if (rawBuf.candOffsets.size() == 0) {
        return 0;
    }

    auto &offsets = rawBuf.candOffsets;
    auto it =
        std::upper_bound(offsets.cbegin(), offsets.cend(), rawBuf.cursor) - 1;

    if (it == offsets.cend()) {
        return offsets.size() - 1;
    }

    return static_cast<size_t>(std::distance(offsets.cbegin(), it));
}

auto BufferManager::empty() -> bool { return rawBuf.text.empty(); }

auto BufferManager::getDispBufAt_(size_t index) -> uint32_t {
    auto it = dispBuf.text.cbegin();
    utf8::advance(it, index, dispBuf.text.cend());
    return utf8::peek_next(it, dispBuf.text.cend());
}

auto BufferManager::getFuzzyCandidates() -> void {
    auto candPos = findCandidateAtCursor();
    auto startCand = candPos > 4 ? candPos - 4 : 0;

    getFuzzyCandidates(startCand);
}

auto BufferManager::getFuzzyCandidates(size_t startCand) -> void {
    auto rawStart = startCand > 0 ? rawBuf.candOffsets[startCand] : 0;
    auto consumed = size_t(0);
    auto idx = 0;
    auto it = primaryCandidate.begin() + startCand;

    while (it != primaryCandidate.end()) {
        it = primaryCandidate.erase(it);
    }

    Candidate lgram;

    if (primaryCandidate.size() > 0) {
        lgram = primaryCandidate[startCand - 1];
    } else {
        lgram = Candidate();
    }

    auto searchStr =
        std::string(rawBuf.text.begin() + rawStart, rawBuf.text.end());

    auto nextCandidate = candidateFinder.findPrimaryCandidate(
        searchStr, toneMode_ == ToneMode::Fuzzy, lgram);

    primaryCandidate.insert(primaryCandidate.end(), nextCandidate.cbegin(),
                            nextCandidate.cend());
}

auto BufferManager::isCursorAtEnd() -> bool {
    return rawBuf.cursor == rawBuf.text.size();
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
    auto prevRawBufLen = rawBuf.text.size();
    rawBuf.text.insert(rawBuf.cursor, 1, ch);
    rawBuf.cursor += 1;

    auto d_begin = dispBuf.text.cbegin();
    auto d_it = dispBuf.ccursorIt();
    auto stayLeft = true;
    if (d_it != d_begin) {
        if (utf8::prior(d_it, dispBuf.text.cbegin()) == ' ') {
            stayLeft = false;
        }
    }

    if (isCursorAtEnd()) {
        getFuzzyCandidates();
    } else {
        getFuzzyCandidates(findCandidateAtCursor());
    }

    updateDisplayBuffer();

    d_begin = dispBuf.text.cbegin();
    d_it = dispBuf.ccursorIt();
    if (d_it != d_begin) {
        if (stayLeft && utf8::prior(d_it, dispBuf.text.cbegin()) == ' ') {
            --dispBuf.cursor;
        }
    }

    return RetVal::TODO;
}

auto BufferManager::insertTelex_(char ch) -> RetVal {
    // TODO: handle telex double press
    // char lk = lastKey_;
    // lastKey_ = ch;

    // Syllable *syl = &syllables_[cursor_.first];
    // size_t *curs = &cursor_.second;
    // bool atEnd = isCursorAtEnd_();

    // Tone tone = getToneFromTelex(ch);
    // tone = checkTone78Swap(syl->unicode, tone);

    // if (lk == ch) {
    //    auto start = syl->display.begin();
    //    auto it = syl->display.end();

    //    if (!atEnd) {
    //        it = start + (*curs);
    //    }

    //    if (ch == 'u' && utf8::prior(it, start) == 0x0358) {
    //        if (atEnd) {
    //            syl->ascii.pop_back();
    //            syl->asciiToUnicodeAndDisplay();
    //            appendNewSyllable_();
    //            syl = &syllables_[cursor_.first];
    //            (*curs) = 0;
    //        }
    //    } else if (tone != Tone::NaT) {
    //        std::string tmp = toNFD(syl->unicode);

    //        if (utf8::prior(tmp.end(), tmp.begin()) ==
    //            ToneToUint32Map.at(tone)) {
    //            syl->ascii.pop_back();
    //            syl->tone = Tone::NaT;
    //            syl->asciiToUnicodeAndDisplay();
    //            appendNewSyllable_();
    //            syl = &syllables_[cursor_.first];
    //            (*curs) = 0;
    //        }
    //    }
    //}

    //// when is tone a tone?
    //// 1. if we are at the end and there isn't already a tone
    //// 2. if we aren't at the end
    // size_t acurs = syl->getAsciiCursor(*curs);
    // bool canPlaceTone =
    //    boost::regex_search(syl->ascii.substr(0, acurs), toneableLetters);

    // if (canPlaceTone && tone != Tone::NaT &&
    //    ((atEnd && syl->tone == Tone::NaT) || (!atEnd))) {
    //    if (tone == Tone::TK) {
    //        if (syl->khin == true) {
    //            // TODO: How to handle if already khin?
    //            return RetVal::TODO;
    //        }

    //        syl->khin = true;

    //        // TODO: how to handle ascii for khin?
    //        syl->asciiToUnicodeAndDisplay();
    //        (*curs)++;

    //        return RetVal::OK;
    //    }

    //    if (!atEnd) {
    //        syl->ascii.pop_back();
    //        // Definitely a tone? Use digit
    //        syl->ascii.push_back(ToneToDigitMap.at(tone));
    //    } else {
    //        syl->ascii.push_back(ch);
    //    }

    //    syl->tone = tone;

    //    auto prevLength = syl->displayUtf8Size();
    //    syl->asciiToUnicodeAndDisplay();
    //    cursor_.second += syl->displayUtf8Size() - prevLength;

    //    return RetVal::OK;
    //}

    // if (atEnd) {
    //    // TODO: check Trie for invalid syllables
    //    if (syl->tone != Tone::NaT) {
    //        appendNewSyllable_();
    //        syl = &syllables_[cursor_.first];
    //        (*curs) = 0;
    //    }
    //    syl->ascii.push_back(ch);
    //    syl->asciiToUnicodeAndDisplay();
    //    (*curs)++;

    //    return RetVal::OK;
    //} else {
    //    syl->ascii.insert(*curs, 1, ch);
    //    syl->asciiToUnicodeAndDisplay();
    //    (*curs)++;

    //    return RetVal::OK;
    //}

    return RetVal::Error;
}

auto BufferManager::removeToneFromRawBuffer() -> void {
    auto sylBegin = rawBuf.sylBegin();
    auto sylIt = rawBuf.sylEnd();
    --sylIt;

    while (sylIt != sylBegin) {
        if (isdigit(*sylIt) && (*sylIt) != '0') {
            if (rawBuf.cursorIt() - 1 == sylIt) {
                rawBuf.cursor--;
            }

            rawBuf.text.erase(sylIt, sylIt + 1);
            break;
        }
        sylIt--;
    }
}

auto BufferManager::updateDisplayBuffer() -> void {
    // editing/normal/fuzzy:
    if (primaryCandidate.empty()) {
        return;
    }

    auto currBufSize = utf8Size(dispBuf.text);

    auto rawSylOffsets = std::vector<size_t>();
    auto rawCandOffsets = std::vector<size_t>();
    auto dbSylOffsets = std::vector<Utf8Size>();
    auto dbCandOffsets = std::vector<Utf8Size>();

    auto rawOffset = size_t(0);
    auto displayOffset = Utf8Size(0);
    auto tmpDispBuf = std::string();

    auto khinNextCand = false;

    // Cases:
    // 1. dictionary entry
    // 2. lomaji, but no dictionary entry
    // 3. dangling hyphen at the end of previous candidate
    // 4. double hyphen (khin) at the beginning of next candidate
    // 5. random/other
    for (const auto &c : primaryCandidate) {
        if (c.dict_id == 0 && c.output.empty()) {
            auto d = std::string(c.ascii);

            if (d.rfind("--", 0) == 0) {
                d.replace(0, 2, U8_TK);
            }

            rawSylOffsets.push_back(rawOffset);
            rawCandOffsets.push_back(rawOffset);
            dbSylOffsets.push_back(displayOffset);
            dbCandOffsets.push_back(displayOffset);
            tmpDispBuf += d + " ";
            displayOffset += utf8Size(d) + 1;
            rawOffset += c.ascii.size();
            continue;
        }

        auto spacedSyls = spaceAsciiByUtf8(c.ascii, c.input);
        auto autokhin = false;

        for (const auto &syl : spacedSyls | boost::adaptors::indexed()) {
            auto &str = syl.value();

            rawSylOffsets.push_back(rawOffset);
            dbSylOffsets.push_back(displayOffset);

            if (syl.index() == 0) {
                rawCandOffsets.push_back(rawOffset);
                dbCandOffsets.push_back(displayOffset);

                if (str.rfind("--", 0) == 0) {
                    autokhin = true;
                }
            } else if (autokhin) {
                str.insert(0, "--");
            }

            rawOffset += str.size();
            auto displaySyl = asciiSyllableToUtf8(str);
            tmpDispBuf += displaySyl;
            displayOffset += utf8Size(displaySyl);

            if (displaySyl.back() != '-') {
                tmpDispBuf += ' ';
                displayOffset += 1;
            }
        }

        autokhin = false;
    }

    if (tmpDispBuf.back() == ' ') {
        tmpDispBuf.pop_back();
    }

    rawBuf.sylOffsets = std::move(rawSylOffsets);
    rawBuf.candOffsets = std::move(rawCandOffsets);
    dispBuf.sylOffsets = std::move(dbSylOffsets);
    dispBuf.candOffsets = std::move(dbCandOffsets);
    dispBuf.text = std::move(tmpDispBuf);

    updateDisplayCursor();

    BOOST_LOG_TRIVIAL(debug) << "IN:  " << rawBuf.text;
    BOOST_LOG_TRIVIAL(debug) << "DB:  " << dispBuf.text;
    auto tmp = std::string();
    for (auto &pc : primaryCandidate) {
        tmp += pc.output;
    }
    BOOST_LOG_TRIVIAL(debug) << "PC:  " << tmp;
}

auto BufferManager::updateDisplayCursor() -> void {
    if (isCursorAtEnd()) {
        dispBuf.cursor =
            utf8::distance(dispBuf.text.begin(), dispBuf.text.end());
    } else {
        auto r_target = rawBuf.cursorIt();
        auto r_it = rawBuf.candBegin();
        auto r_end = rawBuf.text.end();

        auto currCand = findCandidateAtCursor();
        auto d_it = dispBuf.text.begin();
        utf8::advance(d_it, dispBuf.candOffsets[findCandidateAtCursor()],
                      dispBuf.text.end());
        auto d_end = dispBuf.text.end();

        while (r_it != r_end && r_it != r_target && d_it != d_end) {
            parallelNext(r_it, r_end, d_it, d_end);
        }

        dispBuf.cursor = utf8::distance(dispBuf.text.begin(), d_it);
    }
}

} // namespace TaiKey

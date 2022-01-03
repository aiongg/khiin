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
    buffer.clear();

    return RetVal::TODO;
}

auto BufferManager::getDisplayBuffer() -> std::string {
    return buffer.displayText();
}

auto BufferManager::getCursor() -> size_t { return buffer.displayCursorOffset(); }

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
        return insertNormal(ch);
    }

    return RetVal::NotConsumed;
}

auto BufferManager::moveCursor(CursorDirection dir) -> RetVal {
    buffer.moveCursor(dir);
    return RetVal::Consumed;
}

auto BufferManager::erase(CursorDirection dir) -> RetVal {
    buffer.erase(dir);
    replaceCandidates(buffer.segmentAtCursor(), buffer.segmentEnd());
    return RetVal::TODO;
}

auto BufferManager::setToneKeys(ToneKeys toneKeys) -> RetVal {
    toneKeys_ = toneKeys;
    return RetVal::TODO;
}

auto BufferManager::spacebar() -> RetVal {
    if (empty()) {
        return RetVal::NotConsumed;
    }

    return RetVal::TODO;
}

// Private
auto BufferManager::empty() -> bool { return false; /* TODO */ }

auto BufferManager::isCursorAtEnd() -> bool { return buffer.isCursorAtEnd(); }

auto BufferManager::replaceCandidates(SegmentIter first, SegmentIter last)
    -> void {
    SegmentIter begin = buffer.segmentBegin();

    auto lgram =
        first != begin ? buffer.candidateAt(first - 1) : Candidate();

    auto searchStr = buffer.rawText(first, last);
    auto pc = candidateFinder.findPrimaryCandidate(
        searchStr, toneMode == ToneMode::Fuzzy, lgram);

    //auto ac = candidateFinder.findCandidates(
    //    searchStr, toneMode == ToneMode::Fuzzy, lgram);

    buffer.setPrimaryCandidate(first, std::move(pc));
}

auto BufferManager::insertNormal(char ch) -> RetVal {
    SegmentIter begin = buffer.segmentBegin();
    SegmentIter curs = buffer.segmentAtCursor();

    if (buffer.isCursorAtEnd()) {
        if (buffer.segmentCount() > 4) {
            begin = curs - 4;
        } else {
            begin = buffer.editingBegin();
        }
    } else {
        begin = curs;
    }

    buffer.insert(ch);
    replaceCandidates(begin, buffer.segmentEnd());

    return RetVal::TODO;
}

auto BufferManager::insertTelex_(char ch) -> RetVal {

    return RetVal::Error;
}

} // namespace TaiKey

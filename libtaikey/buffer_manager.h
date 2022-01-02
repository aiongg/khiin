#pragma once

#include <string>
#include <vector>

#include "buffer.h"
#include "candidates.h"
#include "common.h"
#include "config.h"
#include "errors.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

using Cursor = std::pair<size_t, size_t>;

/**
 * Select candidate:
 * 1. display, unicode & tone from Candidate
 * 2. check if ascii differs from unicode
 * 3. move extra trailing ascii to next syllable
 * 4. update unicode, display, tone in next syllable
 */

// ascii: chiahj   --> chiah + (extra -> next syl)
// unicode: chia̍h  --> chiah   (input)
// display: 食     --> 者      (input)
// tone: T8        --> T4
//
// ascii: joah
// unicode: joah
// display: joah
// tone: NaT
//
// cursor: <1, 1>

using Hanlo = std::pair<std::string, std::string>;

enum class CursorDirection {
    L,
    R,
};

class BufferManager {
  public:
    BufferManager();
    BufferManager(CandidateFinder &candidateFinder);
    auto clear() -> RetVal;
    auto getDisplayBuffer() -> std::string;
    auto getCursor() -> size_t;
    auto insert(char ch) -> RetVal;
    auto moveCursor(CursorDirection dir) -> RetVal;
    auto remove(CursorDirection dir) -> RetVal;
    auto spacebar() -> RetVal;
    auto selectCandidate(Hanlo candidate);
    auto setToneKeys(ToneKeys toneKeys) -> RetVal;

  private:
    auto atSyllableStart() -> bool;
    auto findCandidateAtCursor() -> size_t;
    auto empty() -> bool;
    auto getDispBufAt_(size_t index) -> uint32_t;
    auto getFuzzyCandidates() -> void;
    auto getFuzzyCandidates(size_t startCand) -> void;
    auto isCursorAtEnd() -> bool;
    auto insertNormal_(char ch) -> RetVal;
    auto insertTelex_(char ch) -> RetVal;
    auto removeToneFromRawBuffer() -> void;
    auto updateDisplayBuffer() -> void;
    auto updateDisplayCursor() -> void;

    Buffer rawBuf;
    Buffer dispBuf; // all measures in utf8 code points

    CandidateFinder &candidateFinder;
    Candidates candidates;
    Cursor cursor_;
    Utf8Size displayCursor_ = 0;
    char lastKey_ = '\0';
    Candidates primaryCandidate;
    size_t rawCursor_ = 0;
    std::vector<int> segmentOffsets_;
    CommitMode commitMode_ = CommitMode::Lazy;
    InputMode inputMode_ = InputMode::Normal;
    ToneKeys toneKeys_ = ToneKeys::Numeric;
    ToneMode toneMode_ = ToneMode::Fuzzy;
};

} // namespace TaiKey

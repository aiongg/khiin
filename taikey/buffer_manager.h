#pragma once

#include <string>
#include <vector>

#include "buffer.h"
#include "candidates.h"
#include "common.h"
#include "config.h"
#include "errors.h"
#include "splitter.h"
#include "trie.h"

namespace taikey {

struct CandidateDisplay {
    std::string text;
    int color = 0;
    std::string hint;
};

class BufferManager {
  public:
    BufferManager();
    BufferManager(CandidateFinder *candidateFinder);
    auto clear() -> RetVal;
    auto empty() -> bool;
    auto focusCandidate(size_t index) -> RetVal;
    auto getCandidates() -> std::vector<CandidateDisplay>;
    auto getDisplayBuffer() -> std::string;
    auto getCursor() -> size_t;
    auto insert(char ch) -> RetVal;
    auto moveCursor(CursorDirection dir) -> RetVal;
    auto moveFocus(CursorDirection dir) -> RetVal;
    auto erase(CursorDirection dir) -> RetVal;
    auto selectPrimaryCandidate() -> RetVal;
    auto selectCandidate(size_t index) -> RetVal;
    auto setToneKeys(ToneKeys toneKeys) -> RetVal;

  private:
    auto isCursorAtEnd() -> bool;
    auto insertNormal(char ch) -> RetVal;
    auto insertTelex_(char ch) -> RetVal;
    auto findCandidates(SegmentIter first) -> void;
    auto findPrimaryCandidate(SegmentIter first, SegmentIter last) -> void;
    auto lgramOf(SegmentIter segment) -> std::string;

    SynchronizedBuffer buffer;
    CandidateFinder *candidateFinder;
    char lastKey_ = '\0';
    CommitMode commitMode_ = CommitMode::Lazy;
    InputMode inputMode_ = InputMode::Normal;
    ToneKeys toneKeys_ = ToneKeys::Numeric;
    ToneMode toneMode = ToneMode::Fuzzy;
    Candidates candidates;
    bool hasPrimaryCandidate = false;
};

} // namespace taikey

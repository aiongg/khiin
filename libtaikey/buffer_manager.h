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

class BufferManager {
  public:
    BufferManager();
    BufferManager(CandidateFinder &candidateFinder);
    auto clear() -> RetVal;
    auto getDisplayBuffer() -> std::string;
    auto getCursor() -> size_t;
    auto insert(char ch) -> RetVal;
    auto moveCursor(CursorDirection dir) -> RetVal;
    auto erase(CursorDirection dir) -> RetVal;
    auto spacebar() -> RetVal;
    auto setToneKeys(ToneKeys toneKeys) -> RetVal;

  private:
    auto empty() -> bool;
    auto isCursorAtEnd() -> bool;
    auto insertNormal(char ch) -> RetVal;
    auto insertTelex_(char ch) -> RetVal;
    auto replaceCandidates(SegmentIter first, SegmentIter last) -> void;

    SynchronizedBuffer buffer;
    CandidateFinder &candidateFinder;
    char lastKey_ = '\0';
    CommitMode commitMode_ = CommitMode::Lazy;
    InputMode inputMode_ = InputMode::Normal;
    ToneKeys toneKeys_ = ToneKeys::Numeric;
    ToneMode toneMode = ToneMode::Fuzzy;
};

} // namespace TaiKey

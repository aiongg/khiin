#pragma once

#include <string>
#include <vector>

#include "candidates.h"
#include "common.h"
#include "config.h"
#include "errors.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

using Cursor = std::pair<size_t, size_t>;

struct Syllable {
    std::string ascii;
    std::string unicode;
    std::string display;
    Tone tone = Tone::NaT;
    bool khin = false;
    auto asciiToUnicodeAndDisplay() -> RetVal;
    auto displayUtf8Size() -> ptrdiff_t;
    auto getAsciiCursor(size_t unicodeCursor) -> ptrdiff_t;
};

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

class Buffer {
  public:
    Buffer();
    // Buffer(std::shared_ptr<Trie> dictTrie, std::shared_ptr<Splitter>
    // sylSplitter);
    Buffer(CandidateFinder &candidateFinder);
    auto getDisplayBuffer() -> std::string;
    auto getCursor() -> size_t;
    auto insert(char ch) -> RetVal;
    auto remove(CursorDirection dir) -> RetVal;
    auto moveCursor(CursorDirection dir) -> RetVal;
    auto clear() -> RetVal;
    auto selectCandidate(Hanlo candidate);
    auto setToneKeys(ToneKeys toneKeys) -> RetVal;

  private:
    auto isCursorAtEnd_() -> bool;
    auto insertNormal_(char ch) -> RetVal;
    auto insertNumeric_(char ch) -> RetVal;
    auto insertTelex_(char ch) -> RetVal;
    auto appendNewSyllable_() -> void;
    auto updateDisplayBuffer_() -> void;

    std::string rawBuffer_;
    std::string displayBuffer_;
    Candidates primaryCandidate_;
    Candidates candidates_;
    std::vector<Syllable> syllables_;
    Cursor cursor_;
    size_t rawCursor_ = 0;
    size_t cursor2_ = 0;
    std::vector<int> segmentOffsets_;
    InputMode inputMode_ = InputMode::Normal;
    ToneKeys toneKeys_ = ToneKeys::Numeric;
    ToneMode toneMode_ = ToneMode::Fuzzy;
    CommitMode commitMode_ = CommitMode::Lazy;
    char lastKey_ = '\0';
    // std::shared_ptr<Trie> dictTrie_;
    // std::shared_ptr<Splitter> sylSplitter_;
    CandidateFinder &candidateFinder_;
};

} // namespace TaiKey

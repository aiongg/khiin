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
    auto selectCandidate(Hanlo candidate);
    auto setToneKeys(ToneKeys toneKeys) -> RetVal;

  private:
    auto appendNewSyllable_() -> void;
    auto findCandidateAtCursor_() -> size_t;
    auto findSyllableBegin_() -> std::pair<size_t, Utf8Size>;
    auto getDispBufAt_(size_t index) -> uint32_t;
    auto getFuzzyCandidates_() -> void;
    auto getFuzzyCandidates_(size_t cursor) -> void;
    auto isCursorAtEnd_() -> bool;
    auto insertNormal_(char ch) -> RetVal;
    auto insertNumeric_(char ch) -> RetVal;
    auto insertTelex_(char ch) -> RetVal;
    auto updateDisplayBuffer_() -> void;

    struct Buffer {
        Buffer()
            : text(""), cursor(0), sylOffsets(std::vector<size_t>()),
              candOffsets(std::vector<size_t>()) {}
        std::string text;
        size_t cursor;
        std::vector<size_t> sylOffsets;
        std::vector<size_t> candOffsets;
    };

    Buffer rawBuf_;
    Buffer dispBuf_; // all measures in utf8 code points

    CandidateFinder &candidateFinder_;
    Candidates candidates_;
    Cursor cursor_;
    Utf8Size displayCursor_ = 0;
    char lastKey_ = '\0';
    Candidates primaryCandidate_;
    size_t rawCursor_ = 0;
    std::vector<int> segmentOffsets_;
    std::vector<Syllable> syllables_;
    CommitMode commitMode_ = CommitMode::Lazy;
    InputMode inputMode_ = InputMode::Normal;
    ToneKeys toneKeys_ = ToneKeys::Numeric;
    ToneMode toneMode_ = ToneMode::Fuzzy;
};

} // namespace TaiKey

#pragma once

#include <string>
#include <vector>

#include "common.h"
#include "errors.h"
#include "trie.h"

namespace TaiKey {

typedef std::pair<size_t, size_t> cursor_t;

struct Syllable {
    std::string ascii;
    std::string unicode;
    std::string display;
    Tone tone = Tone::NaT;
    bool khin = false;
    retval_t asciiToUnicodeAndDisplay();
    size_t displayUtf8Size();
    size_t getAsciiCursor(size_t unicodeCursor);
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

typedef std::pair<std::string, std::string> hanlo_t;

enum class ToneKeys {
    Numeric,
    Telex,
};

enum class CursorDirection {
    L,
    R,
};

class Buffer {
  public:
    Buffer();
    std::string getDisplayBuffer();
    int getCursor();
    retval_t insert(char ch);
    retval_t remove(CursorDirection dir);
    retval_t moveCursor(CursorDirection dir);
    retval_t clear();
    bool selectCandidate(hanlo_t candidate);
    bool setToneKeys(ToneKeys toneKeys);

  private:
    std::vector<Syllable> syllables_;
    cursor_t cursor_;
    std::vector<int> segmentOffsets_;
    ToneKeys toneKeys_;
    bool isCursorAtEnd_();
    retval_t insertNumeric_(char ch);
    retval_t insertTelex_(char ch);
    std::unique_ptr<TNode> sylTrie_;
    char lastKey_;
    void appendNewSyllable_();
};

} // namespace TaiKey

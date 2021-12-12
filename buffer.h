#pragma once

#include <string>
#include <vector>

namespace TaiKey {

typedef std::pair<int, int> cursor_t;

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

enum class Tone {
    NaT,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    T8,
    T9,
    TK,
};

struct Syllable {
    std::string ascii;
    Tone tone;
    std::string unicode; // lomaji, for display before candidate selection
    std::string display; // lomaji or hanji, after candidate selection
};

enum CursorDirection {
    CURS_LEFT,
    CURS_RIGHT,
};

class Buffer {
  public:
    Buffer();
    std::string getDisplayBuffer();
    std::string getCursor();
    bool insert(char ch);
    bool remove(CursorDirection dir);
    bool moveCursor(CursorDirection dir);
    bool selectCandidate(hanlo_t candidate);
    bool setToneKeys(ToneKeys toneKeys);

  private:
    Syllable syllables_[20];
    cursor_t cursor_;
    int segmentOffsets_[20];
    ToneKeys toneKeys_;
};

} // namespace TaiKey

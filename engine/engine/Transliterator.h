#pragma once

#include <string>

namespace khiin::engine {

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

enum class SpecialGlyph {
    Nasal,
    NasalUpper,
    DotAboveRight,
    DotsBelow,
    KhinDot,
};

class Transliterator {
  public:
    static Transliterator *Create();
    virtual std::string Precompose(std::string const &decomposed) = 0;
    virtual std::string Decompose(std::string const &composed) = 0;

    virtual void InsertAt(std::string &composed, int &index, char char_in) = 0;

    virtual void AddConversionRule(std::string input, std::string output) = 0;
    virtual void AddToneKey(Tone tone, char key) = 0;
};

} // namespace khiin::engine

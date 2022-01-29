#pragma once

#include <string>

#include "unicode_utils.h"

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

struct ToneInfo;

class Transliterator {
  public:
    static Transliterator *Create();
    static Transliterator *default_instance();
    virtual std::string Precompose(std::string const &decomposed, ToneInfo const &tone_info) = 0;
    virtual std::string Decompose(std::string const &composed, ToneInfo const &tone_info) = 0;

    //virtual void InsertAt(std::string input, int at_in, char char_in, std::string &output, int &at_out) = 0;

    virtual size_t DecomposedSize(std::string const &composed, utf8_size_t to) = 0;

    virtual void AddConversionRule(std::string input, std::string output) = 0;
    virtual void AddToneKey(Tone tone, char key) = 0;
};

} // namespace khiin::engine

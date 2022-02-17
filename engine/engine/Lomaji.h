#pragma once

#include "common.h"
#include "errors.h"
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

class Lomaji {
  public:
    static utf8_size_t MoveCaret(std::string_view str, utf8_size_t start_pos, CursorDirection dir);
    static inline bool NeedsToneDiacritic(Tone t) {
        return !(t == Tone::NaT || t == Tone::T1 || t == Tone::T4);
    }
    static bool IsLomaji(std::string_view str);
    static std::string Decompose(std::string_view str);
};

} // namespace khiin::engine

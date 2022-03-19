#pragma once

#include "utils/common.h"
#include "utils/errors.h"
#include "utils/unicode.h"

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
    static bool HasToneable(std::string_view str);
    static bool HasToneDiacritic(std::string_view str);
    static utf8_size_t MoveCaret(std::string_view str, utf8_size_t start_pos, CursorDirection dir);
    static inline bool NeedsToneDiacritic(Tone t) {
        return !(t == Tone::NaT || t == Tone::T1 || t == Tone::T4);
    }
    static size_t FindTonePosition(std::string_view syllable);
    static void ApplyToneDiacritic(Tone tone, std::string &syllable);
    static Tone RemoveToneDiacritic(std::string &syllable);
    static bool RemoveKhin(std::string &syllable);
    static void ReplaceKhinDot(std::string &syllable);
    static bool IsLomaji(std::string_view str);
    static std::string Decompose(std::string_view str);

    /**
     * Returns a string copy of |output| such that the casing of the string matches casing
     * provided by |pattern|, irrespective of diacritics, spaces, or hyphens
     */
    static std::string MatchCapitalization(std::string_view pattern, std::string_view output);
};

} // namespace khiin::engine

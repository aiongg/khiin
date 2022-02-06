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

inline const uint32_t U32_NN = 0x207f;
inline const uint32_t U32_OU = 0x0358;
inline const uint32_t U32_T2 = 0x0301;
inline const uint32_t U32_T3 = 0x0300;
inline const uint32_t U32_T5 = 0x0302;
inline const uint32_t U32_T7 = 0x0304;
inline const uint32_t U32_T8 = 0x030D;
inline const uint32_t U32_T9 = 0x0306;
inline const uint32_t U32_TK = 0x00B7;
inline const uint32_t U32_R = 0x0324;
inline const uint32_t U32_NN_UC = 0x1D3A;
inline const uint32_t U32_UR_UC = 0x1E72;
inline const uint32_t U32_UR = 0x1E73;

class Lomaji {
  public:
    static utf8_size_t MoveCaret(std::string_view str, utf8_size_t start_pos, CursorDirection dir);
    static inline bool NeedsToneDiacritic(Tone t) {
        return !(t == Tone::NaT || t == Tone::T1 || t == Tone::T4);
    }
};

auto asciiSyllableToUtf8(std::string ascii) -> std::string;

auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin) -> std::string;

auto checkTone78Swap(std::string u8syllable, Tone tone) -> Tone;

auto getToneFromDigit(char ch) -> Tone;

auto getToneFromTelex(char ch) -> Tone;

auto hasToneDiacritic(std::string sv) -> bool;

auto placeToneOnSyllable(std::string u8syllable, Tone tone) -> std::string;

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji) -> string_vector;

auto tokenSpacer(std::vector<std::string_view> tokens) -> std::vector<bool>;

auto utf8back(std::string_view s) -> uint32_t;

auto utf8first(std::string_view s) -> uint32_t;

auto utf8Size(std::string s) -> utf8_size_t;

auto utf8ToAsciiLower(std::string u8string) -> std::string;

} // namespace khiin::engine

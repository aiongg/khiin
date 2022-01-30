#pragma once

#include <string>

#include "Lomaji.h"
#include "unicode_utils.h"

namespace khiin::engine {

enum class KhinKeyPosition {
    None,
    Start,
    End,
};

struct ToneInfo {
    Tone tone = Tone::NaT;
    char tone_key = 0;
    KhinKeyPosition khin_pos = KhinKeyPosition::None;
    char khin_key = 0;
};

struct Syllable {
    std::string raw_body;
    Tone tone = Tone::NaT;
    KhinKeyPosition khin_pos = KhinKeyPosition::None;
    char tone_key = 0;
    char khin_key = 0;
    std::string composed;
};

} // namespace khiin::engine

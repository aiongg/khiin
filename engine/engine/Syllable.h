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

class SyllableParser;

struct Syllable {
    std::string raw_input;
    std::string raw_body;
    Tone tone = Tone::NaT;
    KhinKeyPosition khin_pos = KhinKeyPosition::None;
    char tone_key = 0;
    char khin_key = 0;
    std::string composed;
    SyllableParser *parser = nullptr;
};

} // namespace khiin::engine

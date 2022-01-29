#pragma once

#include <string>

#include "Transliterator.h"
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

class Syllable {
  public:

  private:
    std::string raw_body;
    ToneInfo tone_info;
};

} // namespace khiin::engine

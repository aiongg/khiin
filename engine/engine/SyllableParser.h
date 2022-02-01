#pragma once

#include "common.h"
#include "unicode_utils.h"

namespace khiin::engine {

class KeyConfig;
struct Syllable;
class BufferSegment;

struct InputSequence {
    std::string input;
    bool is_fuzzy_monosyllable;
};

class SyllableParser {
  public:
    static SyllableParser *Create(KeyConfig *key_config);
    virtual void SetKeyConfiguration(KeyConfig *key_config) = 0;
    virtual void ParseRaw(std::string const &input, Syllable &output) = 0;

    virtual void RawToComposedCaret(Syllable &syllable, size_t raw_caret, utf8_size_t &composed_caret) = 0;
    virtual void ComposedToRawCaret(Syllable &syllable, utf8_size_t composed_caret, size_t &raw_caret) = 0;

    virtual void ToFuzzy(std::string const &input, string_vector &output, bool &has_tone) = 0;
    virtual std::vector<InputSequence> AsInputSequences(std::string const &input) = 0;

    virtual BufferSegment AsBufferSegment(std::string const &raw, std::string const &target) = 0;
};

} // namespace khiin::engine

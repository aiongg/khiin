#pragma once

#include "common.h"
#include "unicode_utils.h"

namespace khiin::engine {

class KeyConfig;
struct Syllable;

class SyllableParser {
  public:
    static SyllableParser *Create(KeyConfig *key_config);
    virtual void SetKeyConfiguration(KeyConfig *key_config) = 0;
    virtual void ParseRaw(std::string const &input, Syllable &output) = 0;
    virtual void ParseRawIndexed(std::string const &input, size_t input_idx, Syllable &output,
                                 utf8_size_t &output_idx) = 0;
    virtual void Compose(Syllable const &input, std::string &output) = 0;
    virtual void ComposeIndexed(Syllable const &input, utf8_size_t input_idx, std::string &output,
                                size_t &output_idx) = 0;

    virtual std::string SerializeRaw(Syllable const &input) = 0;
    virtual void SerializeRaw(Syllable const &input, utf8_size_t caret, std::string &output, size_t &raw_caret) = 0;

    virtual void ToFuzzy(std::string const &input, string_vector &output, bool &has_tone) = 0;
    virtual string_vector GetMultisylInputSequences(std::string const &input) = 0;
};

} // namespace khiin::engine

#pragma once

#include "KeyConfig.h"
#include "unicode_utils.h"
#include "Syllable.h"

namespace khiin::engine {

class SyllableParser {
    void SetKeyConfiguration(KeyConfig *key_config);
    void ParseRaw(std::string const &input, Syllable &output);
    void ParseRawIndexed(std::string const &input, size_t input_idx, Syllable &output, utf8_size_t &output_idx);
    void Compose(Syllable const &input, std::string &output);
    void ComposeIndexed(Syllable const &input, utf8_size_t input_idx, std::string &output, size_t &output_idx);
};

} // namespace khiin::engine

#pragma once

//#include "Syllable.h"
#include "utils/common.h"
#include "utils/unicode.h"

namespace khiin::engine {

class Engine;
class KeyConfig;
struct Syllable;
class TaiText;
enum class KhinKeyPosition;

struct InputSequence {
    std::string input;
    bool is_fuzzy_monosyllable = false;
};

class SyllableParser {
  public:
    static SyllableParser *Create(Engine *engine);
    virtual Syllable ParseRaw(std::string const &input) = 0;
    virtual Syllable ParseComposed(std::string const &input) = 0;
    virtual void ToFuzzy(std::string const &input, std::vector<std::string> &output, bool &has_tone) = 0;
    virtual std::vector<InputSequence> AsInputSequences(std::string const &input) = 0;
    virtual TaiText AsTaiText(std::string const &raw, std::string const &target) = 0;
};

} // namespace khiin::engine

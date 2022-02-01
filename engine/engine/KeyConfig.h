#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include "Lomaji.h"

namespace khiin::engine {

enum class VKey {
    None,
    Nasal,
    DotAboveRight,
    DotsBelow,
    Khin,
    Hyphen,
    TelexTone2,
    TelexTone3,
    TelexTone5,
    TelexTone6,
    TelexTone7,
    TelexTone8,
    TelexTone9,
};

using ConversionRule = std::pair<std::string, std::string>;

struct ConversionRuleSet {
    char key = 0;
    VKey vkey = VKey::None;
    std::vector<ConversionRule> conversion_rules;
};

class KeyConfig {
  public:
    static KeyConfig *CreateEmpty();
    static KeyConfig *Create();
    virtual bool SetKey(char key, VKey vkey, bool standalone = false) = 0;
    virtual std::vector<ConversionRule> ConversionRules() = 0;
    virtual void GetToneKeys(Tone tone, char &digit_key, char &telex_key) = 0;
    virtual Tone CheckToneKey(char ch) = 0;
    virtual void EnableToneDigitFallback(bool enabled) = 0;
};

} // namespace khiin::engine
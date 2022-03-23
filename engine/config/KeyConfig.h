#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "input/Lomaji.h"

namespace khiin {
namespace proto {
class KeyConfiguration;
}

namespace engine {

enum class VKey {
    None,
    Nasal,
    DotAboveRight,
    DotsBelow,
    Hyphen,
    TelexTone2,
    TelexTone3,
    TelexTone5,
    TelexTone6,
    TelexTone7,
    TelexTone8,
    TelexTone9,
    TelexKhin,
};

using ConversionRule = std::pair<std::string, std::string>;

struct ConversionRuleSet {
    char key = 0;
    VKey vkey = VKey::None;
    std::vector<ConversionRule> conversion_rules;
};

class Engine;

class KeyConfig {
  public:
    KeyConfig() = default;
    KeyConfig &operator=(KeyConfig const &) = delete;
    KeyConfig(KeyConfig const &) = delete;
    virtual ~KeyConfig() = 0;

    static std::unique_ptr<KeyConfig> CreateEmpty();
    static std::unique_ptr<KeyConfig> Create();
    static std::unique_ptr<KeyConfig> Create(Engine *engine);

    virtual bool SetKey(char key, VKey vkey, bool standalone = false) = 0;
    virtual std::vector<ConversionRule> const &ConversionRules() = 0;
    virtual std::vector<char> GetHyphenKeys() = 0;
    virtual std::vector<char> GetKhinKeys() = 0;
    virtual bool IsHyphen(char ch) = 0;
    virtual void GetToneKeys(Tone tone, char &digit_key, char &telex_key) = 0;
    virtual Tone CheckToneKey(char ch) = 0;
    virtual bool IsToneKey(unsigned char ch) = 0;
    virtual void EnableToneDigitFallback(bool enabled) = 0;
    virtual std::string Convert(std::string const &str) = 0;
    virtual std::string Deconvert(std::string const &str) = 0;
};

} // namespace engine
} // namespace khiin

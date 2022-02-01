#include "KeyConfig.h"

#include <iterator>

namespace khiin::engine {
namespace {

constexpr char kDisallowedTelexKeys[] = {'a', 'e', 'h', 'i', 'k', 'm', 'n', 'o', 'p', 't', 'u'};
constexpr char kAllowedTelexKeys[] = {'b', 'c', 'd', 'f', 'g', 'j', 'l', 'q', 'r', 's', 'v', 'w', 'x', 'y', 'z'};
constexpr char kAllowedOtherKeys[] = {'d', 'f', 'q', 'r', 'v', 'w', 'x', 'y', 'z'};

static inline const std::string kNasalStr = u8"\u207f";
static inline const std::string kNasalUpperStr = u8"\u1d3a";
static inline const std::string kODotStr = u8"o\u0358";
static inline const std::string kODotUpperStr = u8"O\u0358";
static inline const std::string kODotsBelowStr = u8"o\u0324";
static inline const std::string kODotsBelowUpperStr = u8"O\u0324";
static inline const std::string kUDotsBelowStr = u8"u\u0324";
static inline const std::string kUDotsBelowUpperStr = u8"U\u0324";

static inline bool is_allowed_other_key(char key) {
    for (auto &c : kAllowedOtherKeys) {
        if (key == c) {
            return true;
        }
    }

    return false;
}

static inline bool is_allowed_nasal_key(char key, bool standalone) {
    if (!is_allowed_other_key(key)) {
        if (standalone || key != 'n') {
            return false;
        }
    }
    return true;
}

static inline bool is_allowed_dotaboveright_key(char key, bool standalone) {
    if (!is_allowed_other_key(key)) {
        if (standalone || (key != 'o' && key != 'u')) {
            return false;
        }
    }
    return true;
}

class KeyCfgImpl : public KeyConfig {
  public:
    virtual bool SetKey(char key, VKey vkey, bool standalone = false) override {
        switch (vkey) {
        case VKey::Nasal:
            return SetNasal(key, standalone);
        case VKey::DotAboveRight:
            return SetDotAboveRight(key, standalone);
        case VKey::DotsBelow:
            return SetDotsBelow(key);
        default:
            return false;
        }
    }

    virtual std::vector<ConversionRule> ConversionRules() override {
        auto ret = std::vector<ConversionRule>();
        for (auto &rule : m_conversion_rule_sets) {
            std::copy(rule.conversion_rules.begin(), rule.conversion_rules.end(), std::back_inserter(ret));
        }
        return ret;
    }

    virtual void GetToneKeys(Tone tone, char &digit_key, char &telex_key) override {
        // TODO: Handle telex keys
        switch (tone) {
        case Tone::T2:
            digit_key = '2';
            break;
        case Tone::T3:
            digit_key = '3';
            break;
        case Tone::T5:
            digit_key = '5';
            break;
        case Tone::T7:
            digit_key = '7';
            break;
        case Tone::T8:
            digit_key = '8';
            break;
        case Tone::T9:
            digit_key = '9';
            break;
        }
    }

    virtual Tone CheckToneKey(char ch) override {
        // TODO: Handle telex keys and fallback to digits

        switch (ch) {
        case '2':
            return Tone::T2;
        case '3':
            return Tone::T3;
        case '5':
            return Tone::T5;
        case '7':
            return Tone::T7;
        case '8':
            return Tone::T8;
        case '9':
            return Tone::T9;
        default:
            return Tone::NaT;
        }
    }

    virtual void EnableToneDigitFallback(bool enabled) {
        return;
    }

  private:
    bool SetNasal(char key, bool standalone) {
        if (!is_allowed_nasal_key(key, standalone)) {
            return false;
        }
        if (!is_key_available(key, VKey::Nasal)) {
            return false;
        }

        auto &rule_set = GetRuleSet(key, VKey::Nasal);
        m_key_map[VKey::Nasal] = key;
        standalone_nasal = standalone;

        auto key_lc = std::string(1, key);
        auto key_uc = std::string(1, toupper(key));

        if (standalone) {
            rule_set.conversion_rules.push_back(std::make_pair(key_lc, kNasalStr));
            rule_set.conversion_rules.push_back(std::make_pair(key_uc, kNasalUpperStr));
        } else {
            rule_set.conversion_rules.push_back(std::make_pair("n" + key_lc, kNasalStr));
            rule_set.conversion_rules.push_back(std::make_pair("n" + key_uc, kNasalStr));
            rule_set.conversion_rules.push_back(std::make_pair("N" + key_lc, kNasalUpperStr));
            rule_set.conversion_rules.push_back(std::make_pair("N" + key_uc, kNasalUpperStr));
        }

        return true;
    }

    bool SetDotAboveRight(char key, bool standalone) {
        if (!is_allowed_dotaboveright_key(key, standalone)) {
            return false;
        }
        if (!is_key_available(key, VKey::DotAboveRight)) {
            return false;
        }

        auto &rule_set = GetRuleSet(key, VKey::DotAboveRight);
        m_key_map[VKey::DotAboveRight] = key;
        standalone_dotaboveright = standalone;

        auto key_lc = std::string(1, key);
        auto key_uc = std::string(1, toupper(key));

        if (standalone) {
            rule_set.conversion_rules.push_back(std::make_pair(key_lc, kODotStr));
            rule_set.conversion_rules.push_back(std::make_pair(key_uc, kODotUpperStr));
        } else {
            rule_set.conversion_rules.push_back(std::make_pair("o" + key_lc, kODotStr));
            rule_set.conversion_rules.push_back(std::make_pair("o" + key_uc, kODotStr));
            rule_set.conversion_rules.push_back(std::make_pair("O" + key_lc, kODotUpperStr));
            rule_set.conversion_rules.push_back(std::make_pair("O" + key_uc, kODotUpperStr));
        }

        return true;
    }

    bool SetDotsBelow(char key) {
        if (!is_allowed_other_key(key)) {
            return false;
        }
        if (!is_key_available(key, VKey::DotsBelow)) {
            return false;
        }

        auto &rule_set = GetRuleSet(key, VKey::DotsBelow);
        m_key_map[VKey::DotsBelow] = key;
        standalone_dotsbelow = false;

        auto key_lc = std::string(1, key);
        auto key_uc = std::string(1, toupper(key));

        rule_set.conversion_rules.push_back(std::make_pair("o" + key_lc, kODotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("o" + key_uc, kODotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("O" + key_lc, kODotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("O" + key_uc, kODotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("u" + key_lc, kUDotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("u" + key_uc, kUDotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("U" + key_lc, kUDotsBelowStr));
        rule_set.conversion_rules.push_back(std::make_pair("U" + key_uc, kUDotsBelowStr));

        return true;
    }

    ConversionRuleSet &GetRuleSet(char key, VKey vkey) {
        auto found =
            std::find_if(m_conversion_rule_sets.begin(), m_conversion_rule_sets.end(), [&](ConversionRuleSet ea) {
                return ea.vkey == vkey;
            });

        if (found == m_conversion_rule_sets.end()) {
            m_conversion_rule_sets.push_back(ConversionRuleSet());
            found = m_conversion_rule_sets.end() - 1;
        }

        auto &rule_set = *found;
        rule_set.vkey = vkey;
        rule_set.key = key;
        return rule_set;
    }

    inline bool is_key_available(char key, VKey vkey) {
        for (auto &[used_vkey, used_key] : m_key_map) {
            if (key == used_key && vkey != used_vkey) {
                return false;
            }
        }

        return true;
    }

    std::vector<ConversionRuleSet> m_conversion_rule_sets;
    std::unordered_map<VKey, char> m_key_map;
    bool standalone_nasal = false;
    bool standalone_dotaboveright = false;
    bool standalone_dotsbelow = false;
    bool use_fallback_tone_digits = true;
};

} // namespace

KeyConfig *KeyConfig::Create() {
    auto key_config = new KeyCfgImpl();
    key_config->SetKey('n', VKey::Nasal);
    key_config->SetKey('u', VKey::DotAboveRight);
    key_config->SetKey('r', VKey::DotsBelow);
    return key_config;
}

KeyConfig *KeyConfig::CreateEmpty() {
    return new KeyCfgImpl();
}

} // namespace khiin::engine
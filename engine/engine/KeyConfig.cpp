#include "KeyConfig.h"

namespace khiin::engine {
namespace {

constexpr char kDisallowedTelexKeys[] = {'a', 'e', 'h', 'i', 'k', 'm', 'n', 'o', 'p', 't', 'u'};
constexpr char kAllowedTelexKeys[] = {'b', 'c', 'd', 'f', 'g', 'j', 'l', 'q', 'r', 's', 'v', 'w', 'x', 'y', 'z'};
constexpr char kAllowedOtherKeys[] = {'d', 'f', 'q', 'r', 'v', 'w', 'x', 'y', 'z'};

static inline const std::string kNasalStr = u8"\u207f";
static inline const std::string kNasalUpperStr = u8"\u1d3a";
static inline const std::string kODotStr = u8"o\u0358";
static inline const std::string kODotUpperStr = u8"O\u0358";

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

    virtual Tone CheckToneKey(char ch) override {
        return Tone();
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

        if (standalone) {
            rule_set.conversion_rules.push_back(std::make_pair(std::string(1, key), kNasalStr));
            rule_set.conversion_rules.push_back(std::make_pair(std::string(1, toupper(key)), kNasalUpperStr));
        } else {
            auto a = std::string("n");
            auto b = std::string("n");
            auto c = std::string("N");
            auto d = std::string("N");
            a.push_back(key);
            b.push_back(toupper(key));
            c.push_back(key);
            d.push_back(toupper(key));
            rule_set.conversion_rules.push_back(std::make_pair(a, kNasalStr));
            rule_set.conversion_rules.push_back(std::make_pair(b, kNasalStr));
            rule_set.conversion_rules.push_back(std::make_pair(c, kNasalUpperStr));
            rule_set.conversion_rules.push_back(std::make_pair(d, kNasalUpperStr));
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
        standalone_nasal = standalone;

        if (standalone) {
            rule_set.conversion_rules.push_back(std::make_pair(std::string(1, key), kODotStr));
            rule_set.conversion_rules.push_back(std::make_pair(std::string(1, toupper(key)), kODotUpperStr));
        } else {
            auto a = std::string("o");
            auto b = std::string("o");
            auto c = std::string("O");
            auto d = std::string("O");
            a.push_back(key);
            b.push_back(toupper(key));
            c.push_back(key);
            d.push_back(toupper(key));
            rule_set.conversion_rules.push_back(std::make_pair(a, kODotStr));
            rule_set.conversion_rules.push_back(std::make_pair(b, kODotStr));
            rule_set.conversion_rules.push_back(std::make_pair(c, kODotUpperStr));
            rule_set.conversion_rules.push_back(std::make_pair(d, kODotUpperStr));
        }

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
};

} // namespace

KeyConfig *KeyConfig::Create() {
    auto key_config = new KeyCfgImpl();
    key_config->SetKey('n', VKey::Nasal);
    key_config->SetKey('u', VKey::DotAboveRight);
    return key_config;
}

KeyConfig *KeyConfig::CreateEmpty() {
    return new KeyCfgImpl();
}

} // namespace khiin::engine

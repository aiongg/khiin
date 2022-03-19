#include "KeyConfig.h"

#include <iterator>

#include "Config.h"
#include "Engine.h"

namespace khiin::engine {
namespace {

using namespace khiin::proto;

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

constexpr char kDefaultNasal = 'n';

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

class KeyCfgImpl : public KeyConfig, ConfigChangeListener {
  public:
    KeyCfgImpl(Engine *engine) : m_engine(engine) {
        if (m_engine) {
            m_engine->RegisterConfigChangedListener(this);
        }
    };

    // Inherited via ConfigChangeListener
    virtual void OnConfigChanged(Config *config) {
        ReloadEngineConfig();
    }

  private:
    void Reset() {
        m_conversion_rule_cache.clear();
        m_conversion_rule_sets.clear();
        m_key_map.clear();
        standalone_nasal = false;
        standalone_dotaboveright = false;
        use_fallback_tone_digits = true;
    }

    void ReloadDefaultConfig() {
        Reset();
        SetKey('n', VKey::Nasal);
        SetKey('u', VKey::DotAboveRight);
        SetKey('r', VKey::DotsBelow);
    }

    void ReloadEngineConfig() {
        if (m_engine == nullptr) {
            ReloadDefaultConfig();
            return;
        }

        Reset();
        auto config = m_engine->config();
       
        if (auto key = config->nasal(); key.size() == 1) {
            SetKey(tolower(key.front()), VKey::Nasal, true);
        } else if (key.size() == 2 && tolower(key.front()) == 'n') {
            SetKey(tolower(key.back()), VKey::Nasal, false);
        } else {
            SetKey('n', VKey::Nasal, false);
        }

        if (auto key = config->dot_above_right(); key.size() == 1) {
            SetKey(tolower(key.front()), VKey::DotAboveRight, true);
        } else if (key.size() == 2 && tolower(key.front()) == 'o') {
            SetKey(tolower(key.back()), VKey::DotAboveRight, false);
        } else {
            SetKey('u', VKey::DotAboveRight, false);
        }

        if (auto key = config->dots_below(); key != 0) {
            SetKey(tolower(key), VKey::DotsBelow, false);
        } else {
            SetKey('r', VKey::DotsBelow);
        }
    }

    bool SetKey(char key, VKey vkey, bool standalone = false) override {
        auto set = false;
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

    std::vector<ConversionRule> const &ConversionRules() override {
        if (m_conversion_rule_cache.empty()) {
            for (auto &rule : m_conversion_rule_sets) {
                std::copy(rule.conversion_rules.begin(), rule.conversion_rules.end(),
                          std::back_inserter(m_conversion_rule_cache));
            }
        }
        return m_conversion_rule_cache;
    }

    std::string Convert(std::string const &input) override {
        std::string ret = input;
        auto &rules = ConversionRules();

        for (auto &rule : rules) {
            if (auto pos = ret.find(rule.first); pos != std::string::npos) {
                ret.replace(pos, rule.first.size(), rule.second);
            }
        }
        return ret;
    }

    std::string Deconvert(std::string const &input) override {
        std::string ret = input;
        auto &rules = ConversionRules();

        for (auto &rule : rules) {
            if (auto pos = ret.find(rule.second); pos != std::string::npos) {
                ret.replace(pos, rule.second.size(), rule.first);
            }
        }
        return ret;
    }

    std::vector<char> GetHyphenKeys() override {
        auto ret = std::vector<char>();
        ret.push_back('-');
        if (auto it = m_key_map.find(VKey::Hyphen); it != m_key_map.end()) {
            ret.push_back(it->second);
        }
        return ret;
    }

    std::vector<char> GetKhinKeys() override {
        auto ret = std::vector<char>();
        ret.push_back('0');
        if (auto it = m_key_map.find(VKey::TelexKhin); it != m_key_map.end()) {
            ret.push_back(it->second);
        }
        return ret;
    }

    void GetToneKeys(Tone tone, char &digit_key, char &telex_key) override {
        // TODO: Handle telex keys
        switch (tone) {
        case Tone::T1:
            digit_key = '1';
            break;
        case Tone::T2:
            digit_key = '2';
            break;
        case Tone::T3:
            digit_key = '3';
            break;
        case Tone::T4:
            digit_key = '4';
            break;
        case Tone::T5:
            digit_key = '5';
            break;
        case Tone::T6:
            digit_key = '6';
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

    Tone CheckToneKey(char ch) override {
        // TODO: Handle telex keys and fallback to digits

        switch (ch) {
        case '1':
            return Tone::T1;
        case '2':
            return Tone::T2;
        case '3':
            return Tone::T3;
        case '4':
            return Tone::T4;
        case '5':
            return Tone::T5;
        case '6':
            return Tone::T6;
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

    bool IsToneKey(char ch) override {
        // TODO: handle telex keys
        return isdigit(ch);
    }

    void EnableToneDigitFallback(bool enabled) override {
        return;
    }

    bool IsHyphen(char ch) override {
        auto hyphen_keys = GetHyphenKeys();
        for (auto key : hyphen_keys) {
            if (ch == key) {
                return true;
            }
        }
        return false;
    }

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

    void ClearKey(char key) {
        auto vkey = VKey::None;

        for (auto &[vk, ch] : m_key_map) {
            if (key == ch) {
                vkey = vk;
            }
        }

        if (vkey == VKey::None) {
            return;
        }

        if (auto it = m_key_map.find(vkey); it != m_key_map.end()) {
            m_key_map.erase(it);
        }
        m_conversion_rule_sets.erase(
            std::remove_if(m_conversion_rule_sets.begin(), m_conversion_rule_sets.end(), [&](ConversionRuleSet &ea) {
                return ea.vkey == vkey;
            }));
    }

    bool is_key_available(char key, VKey vkey) {
        for (auto &[used_vkey, used_key] : m_key_map) {
            if (key == used_key && vkey != used_vkey) {
                return false;
            }
        }

        return true;
    }

    Engine *m_engine = nullptr;
    std::vector<ConversionRule> m_conversion_rule_cache;
    std::vector<ConversionRuleSet> m_conversion_rule_sets;
    std::unordered_map<VKey, char> m_key_map;
    bool standalone_nasal = false;
    bool standalone_dotaboveright = false;
    bool use_fallback_tone_digits = true;
};

} // namespace

KeyConfig *KeyConfig::Create() {
    auto key_config = reinterpret_cast<KeyConfig *>(new KeyCfgImpl(nullptr));
    key_config->SetKey('n', VKey::Nasal);
    key_config->SetKey('u', VKey::DotAboveRight);
    key_config->SetKey('r', VKey::DotsBelow);
    return key_config;
}

KeyConfig *KeyConfig::Create(Engine *engine) {
    return new KeyCfgImpl(engine);
}

KeyConfig *KeyConfig::CreateEmpty() {
    return new KeyCfgImpl(nullptr);
}

} // namespace khiin::engine

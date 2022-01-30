#include "SyllableParser.h"

#include "KeyConfig.h"
#include "Syllable.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {

// Merges sets of strings using an "odometer-like" wrapping iterator
template <typename T>
std::vector<T> odometer_merge(std::vector<std::vector<T>> const &vector_set) {
    auto size = vector_set.size();
    auto ret = std::vector<T>();
    auto its = std::vector<std::vector<T>::const_iterator>();
    for (auto &vec : vector_set) {
        its.push_back(vec.cbegin());
    }

    while (its[0] != vector_set[0].cend()) {
        auto el = T();
        for (auto &it : its) {
            el.append(*it);
        }
        ret.push_back(std::move(el));

        // reset any iterators that have reached the end
        ++its[size - 1];
        for (size_t i = size - 1; (i > 0) && (its[i] == vector_set[i].cend()); --i) {
            its[i] = vector_set[i].cbegin();
            ++its[i - 1];
        }
    }

    return ret;
}

inline const char32_t kNasalLower = 0x207f;
inline const char32_t kNasalUpper = 0x1d3a;
inline const char32_t kDotAboveRight = 0x0358;
inline const char32_t kDotsBelow = 0x0324;
inline const char32_t kKhinDot = 0x00b7;
inline const char32_t kTone2 = 0x0301;
inline const char32_t kTone3 = 0x0300;
inline const char32_t kTone5 = 0x0302;
inline const char32_t kTone7 = 0x0304;
inline const char32_t kTone8 = 0x030d;
inline const char32_t kTone9 = 0x0306;

inline const std::vector<std::string> kOrderedToneableCombinations = {"OA", "Oa", "oa", "OE", "Oe", "oe"};

inline constexpr char kOrderedToneableLetters[] = {'O', 'o', 'A', 'a', 'E', 'e', 'U',
                                                   'u', 'I', 'i', 'M', 'm', 'N', 'n'};

inline constexpr char kSyllableSeparator[] = {' ', '-'};

const std::unordered_map<Tone, std::string> kToneToUnicodeMap = {
    {Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"}, {Tone::T5, u8"\u0302"}, {Tone::T7, u8"\u0304"},
    {Tone::T8, u8"\u030d"}, {Tone::T9, u8"\u0306"}, {Tone::TK, u8"\u00b7"}};

const std::string kKhinDotStr = u8"\u00b7";

inline bool IsSyllableSeparator(char ch) {
    return ch == '-' || ch == ' ';
}

bool HasToneable(std::string_view str) {
    for (auto &c1 : str) {
        for (auto &c2 : kOrderedToneableLetters) {
            if (c1 == c2) {
                return true;
            }
        }
    }

    return false;
}

bool FindTonePosition(std::string_view str, size_t &position) {
    for (auto &sequence : kOrderedToneableCombinations) {
        if (auto i = str.find(sequence); i != std::string::npos) {
            position = i + 2;
            return true;
        }
    }

    for (auto &letter : kOrderedToneableLetters) {
        if (auto i = str.find(letter); i != std::string::npos) {
            position = i + 1;
            return true;
        }
    }

    return false;
}

bool RemoveToneChar(KeyConfig *keyconfig, std::string &input, Tone &tone, char &tone_key) {
    if (!HasToneable(input)) {
        return false;
    }

    tone = keyconfig->CheckToneKey(input.back());
    if (tone == Tone::NaT) {
        return false;
    }

    tone_key = input.back();
    input.pop_back();
    return true;
}

void RemoveToneDiacritic(std::string &input) {
    for (auto &[t, t_str] : kToneToUnicodeMap) {
        if (auto i = input.find(t_str); i != std::string::npos) {
            input.erase(i, t_str.size());
            return;
        }
    }
}

bool RemoveToneDiacriticSaveTone(KeyConfig *keyconfig, std::string &input, Tone &tone, char &key, char &telex_key) {
    for (auto &[t, t_str] : kToneToUnicodeMap) {
        if (auto i = input.find(t_str); i != std::string::npos) {
            tone = t;
            input.erase(i, t_str.size());
            keyconfig->GetToneKeys(tone, key, telex_key);
            return true;
        }
    }

    return false;
}

void ApplyConversionRules(KeyConfig *keyconfig, std::string const &input, std::string &output) {
    output = input;
    auto &rules = keyconfig->ConversionRules();

    for (auto &rule : rules) {
        if (auto pos = output.find(rule.first); pos != std::string::npos) {
            output.replace(pos, rule.first.size(), rule.second);
        }
    }
}

void ApplyDeconversionRules(KeyConfig *keyconfig, std::string &syllable) {
    auto &rules = keyconfig->ConversionRules();

    for (auto &rule : rules) {
        if (auto pos = syllable.find(rule.second); pos != std::string::npos) {
            syllable.replace(pos, rule.second.size(), rule.first);
        }
    }
}

bool AddToneDiacritic(Tone tone, std::string &toneless) {
    size_t tone_index = 0;
    auto found = FindTonePosition(toneless, tone_index);
    if (!found) {
        return false;
    }
    toneless.insert(tone_index, kToneToUnicodeMap.at(tone));
    return true;
}

void EraseAllKhin(std::string &input) {
    auto idx = std::string::npos;
    while ((idx = input.find(kKhinDotStr)) != std::string::npos) {
        input.erase(idx, kKhinDotStr.size());
    }
}

class SyllableParserImpl : public SyllableParser {
  public:
    SyllableParserImpl(KeyConfig *key_config) : keyconfig(key_config) {}

    virtual void SetKeyConfiguration(KeyConfig *key_config) override {
        keyconfig = key_config;
    }

    virtual void ParseRaw(std::string const &input, Syllable &output) override {
        output.parser = this;
        output.raw_body = input;
        RemoveToneChar(keyconfig, output.raw_body, output.tone, output.tone_key);
        ApplyConversionRules(keyconfig, output.raw_body, output.composed);
        if (!AddToneDiacritic(output.tone, output.composed)) {
            output.composed.push_back(output.tone_key);
        }
        output.composed = toNFC(output.composed);
    }

    virtual void ParseRawIndexed(std::string const &input, size_t input_idx, Syllable &output,
                                 utf8_size_t &output_idx) override {}

    virtual void Compose(Syllable const &input, std::string &output) override {}

    virtual void ComposeIndexed(Syllable const &input, utf8_size_t input_idx, std::string &output,
                                size_t &output_idx) override {}

    virtual std::string SerializeRaw(Syllable const &input) override {
        // TODO handle khin
        std::string ret = input.raw_body;
        if (input.tone != Tone::NaT) {
            ret += input.tone_key;
        }
        return ret;
    };

    virtual void SerializeRaw(Syllable const &input, utf8_size_t caret, std::string &output,
                              size_t &raw_caret) override {
        auto &syl = input.composed;
        auto it = syl.cbegin();
        utf8::unchecked::advance(it, caret);

        if (it == syl.cend()) {
            output = SerializeRaw(input);
            raw_caret = output.size();
            return;
        }

        auto lhs = toNFD(std::string(syl.cbegin(), it));
        auto rhs = toNFD(std::string(it, syl.cend()));

        RemoveToneDiacritic(lhs);
        RemoveToneDiacritic(rhs);
        ApplyDeconversionRules(keyconfig, lhs);
        ApplyDeconversionRules(keyconfig, rhs);

        // TODO: Handle khin (on the left before calling lhs.size())

        raw_caret = lhs.size();
        output.append(lhs);
        output.append(rhs);
        if (input.tone != Tone::NaT) {
            output.push_back(input.tone_key);
        }
    }

    virtual void ToFuzzy(const std::string &input, string_vector &output, bool &has_tone) {
        auto syl = toNFD(input);
        auto tone = Tone::NaT;
        char digit_key = 0;
        char telex_key = 0;
        auto found_tone = RemoveToneDiacriticSaveTone(keyconfig, syl, tone, digit_key, telex_key);
        ApplyDeconversionRules(keyconfig, syl);

        if (!found_tone) {
            has_tone = false;
            output.push_back(std::move(syl));
            return;
        }

        has_tone = true;
        syl.push_back(digit_key);
        output.push_back(syl);

        if (telex_key) {
            syl.pop_back();
            syl.push_back(telex_key);
            output.push_back(syl);
        }
    }

    virtual string_vector GetMultisylInputSequences(std::string const &word) override {
        std::string word_copy = word;
        EraseAllKhin(word_copy);
        auto start = word_copy.cbegin();
        auto end = word_copy.cend();

        auto chunks = std::vector<std::vector<std::string>>();
        auto sep = std::find_if(start, end, IsSyllableSeparator);
        auto has_tone = false;

        auto push_chunk = [&](auto from, auto to, bool allow_toneless) {
            auto syl = std::string(from, to);
            auto tmp = std::vector<std::string>();
            ToFuzzy(syl, tmp, has_tone);
            if (has_tone && allow_toneless) {
                tmp.push_back(std::string(tmp[0].cbegin(), tmp[0].cend() - 1));
            }
            chunks.push_back(std::move(tmp));
            has_tone = false;
        };

        auto push_delim = [&](char ch) {
            chunks.push_back(std::vector<std::string>{std::string(1, ch), std::string()});
        };

        while (sep != end) {
            push_chunk(start, sep, true);
            push_delim(*sep);
            start = sep + 1;
            sep = std::find_if(start, end, IsSyllableSeparator);
        }

        if (chunks.size() == 0) {
            push_chunk(start, sep, false);
        } else {
            push_chunk(start, sep, true);
        }

        if (chunks.size() == 1) {
            return chunks[0];
        }

        return odometer_merge(chunks);
    }

  private:
    KeyConfig *keyconfig;
};

} // namespace

SyllableParser *SyllableParser::Create(KeyConfig *key_config) {
    return new SyllableParserImpl(key_config);
}

} // namespace khiin::engine

#include "SyllableParser.h"

#include <algorithm>

#include "KeyConfig.h"
#include "Syllable.h"
#include "TaiText.h"
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
const std::string kSpaceStr = u8" ";

void ToLower(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
}

inline bool IsSyllableSeparator(char ch) {
    return ch == '-' || ch == ' ';
}

inline bool EndsWithPtkh(std::string_view str) {
    auto ch = str.back();
    return ch == 'p' || ch == 't' || ch == 'k' || ch == 'h';
}

inline bool NeedsToneDiacritic(Tone t) {
    return !(t == Tone::NaT || t == Tone::T1 || t == Tone::T4);
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
        if (auto i = str.find(sequence); i != std::string::npos && str.size() > i + 2) {
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
    auto nfd = toNFD(input);
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

std::string ApplyConversionRules(KeyConfig *keyconfig, std::string const &input) {
    auto output = std::string(input);
    auto &rules = keyconfig->ConversionRules();

    for (auto &rule : rules) {
        if (auto pos = output.find(rule.first); pos != std::string::npos) {
            output.replace(pos, rule.first.size(), rule.second);
        }
    }

    return output;
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
    if (tone == Tone::NaT) {
        return false;
    }

    size_t tone_index = 0;
    auto found = FindTonePosition(toneless, tone_index);
    if (!found) {
        return false;
    }
    toneless.insert(tone_index, kToneToUnicodeMap.at(tone));
    return true;
}

void EraseAll(std::string &input, std::string const &substr) {
    auto idx = std::string::npos;
    while ((idx = input.find(substr)) != std::string::npos) {
        input.erase(idx, substr.size());
    }
}

void EraseAllKhin(std::string &input) {
    auto idx = std::string::npos;
    while ((idx = input.find(kKhinDotStr)) != std::string::npos) {
        input.erase(idx, kKhinDotStr.size());
    }
}

Syllable SylFromRaw(KeyConfig *keyconfig, std::string const &input) {
    auto output = Syllable();
    output.raw_input = input;
    output.raw_body = input;
    RemoveToneChar(keyconfig, output.raw_body, output.tone, output.tone_key);
    output.composed = ApplyConversionRules(keyconfig, output.raw_body);
    if (NeedsToneDiacritic(output.tone)) {
        AddToneDiacritic(output.tone, output.composed);
    }
    output.composed = toNFC(output.composed);
    return output;
}

std::string SylToRaw(Syllable const &input) {
    // TODO handle khin
    std::string ret = input.raw_body;
    if (input.tone != Tone::NaT) {
        ret += input.tone_key;
    }
    return ret;
};

Syllable SylFromComposed(KeyConfig *keyconfig, std::string const &input) {
    auto copy = toNFD(input);
    auto tone = Tone::NaT;
    char digit_key = 0;
    char telex_key = 0;
    RemoveToneDiacriticSaveTone(keyconfig, copy, tone, digit_key, telex_key);
    ApplyDeconversionRules(keyconfig, copy);
    auto syl = Syllable();
    syl.composed = input;
    syl.raw_body = copy;
    syl.tone = tone;
    syl.tone_key = digit_key;
    syl.raw_input = SylToRaw(syl);
    return syl;
}

// |raw| may be more than one syllable, and may have separators or tones or not.
// |target| is exactly one syllable.
Syllable AlignRawToComposed(KeyConfig *keyconfig, std::string::const_iterator &r_begin,
                            std::string::const_iterator const &r_end, std::string const &target) {
    auto compare = SylFromComposed(keyconfig, target);

    auto r_it = r_begin;
    auto t_it = compare.raw_input.cbegin();
    auto t_end = compare.raw_input.cend();

    while (r_it != r_end && t_it != t_end && *r_it == *t_it) {
        ++r_it;
        ++t_it;
    }

    if (r_it != r_end && compare.tone != Tone::NaT) {
        auto r_maybe_tone = keyconfig->CheckToneKey(*r_it);
        if (r_maybe_tone != compare.tone && r_it != r_end) {
            r_maybe_tone = keyconfig->CheckToneKey(r_it[1]);
            if (r_maybe_tone == compare.tone) {
                ++r_it;
            }
        }
    }

    auto r_syl = std::string(r_begin, r_it);
    r_begin = r_it;
    return SylFromRaw(keyconfig, r_syl);
}

void ComposedToRawWithAlternates(KeyConfig *keyconfig, const std::string &input, string_vector &output,
                                 bool &has_tone) {
    auto syl = toNFD(input);
    auto tone = Tone::NaT;
    char digit_key = 0;
    char telex_key = 0;
    auto found_tone = RemoveToneDiacriticSaveTone(keyconfig, syl, tone, digit_key, telex_key);
    ApplyDeconversionRules(keyconfig, syl);
    ToLower(syl);

    if (!found_tone) {
        has_tone = false;
        output.push_back(syl);

        if (EndsWithPtkh(syl)) {
            syl.push_back('4');
        } else {
            syl.push_back('1');
        }

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

utf8_size_t RawCaretToComposedCaret(KeyConfig *keyconfig, Syllable &syllable, size_t raw_caret) {
    auto const &input = syllable.raw_input;
    auto const &body = syllable.raw_body;
    auto ret = std::string::npos;

    if (raw_caret == input.size()) {
        ret = Utf8Size(syllable.composed);
    } else if (raw_caret < input.size()) {
        auto lhs = std::string(body.cbegin(), body.cbegin() + raw_caret);
        lhs = ApplyConversionRules(keyconfig, lhs);

        if (syllable.tone != Tone::NaT) {
            size_t tone_pos = std::string::npos;
            FindTonePosition(body, tone_pos);
            if (tone_pos <= lhs.size()) {
                AddToneDiacritic(syllable.tone, lhs);
            }
        }
        ret = Utf8Size(toNFC(lhs));
    }

    return ret;
}

size_t ComposedCaretToRawCaret(KeyConfig *keyconfig, Syllable &syllable, utf8_size_t composed_caret) {
    auto size = utf8::distance(syllable.composed.cbegin(), syllable.composed.cend());
    auto ret = std::string::npos;

    if (composed_caret == size) {
        ret = syllable.raw_input.size();
    } else if (composed_caret < size) {
        auto caret = syllable.composed.cbegin();
        utf8::unchecked::advance(caret, composed_caret);
        auto lhs = toNFD(std::string(syllable.composed.cbegin(), caret));

        RemoveToneDiacritic(lhs);
        ApplyDeconversionRules(keyconfig, lhs);
        ret = lhs.size();
    }

    return ret;
}

class SyllableParserImpl : public SyllableParser {
  public:
    SyllableParserImpl(KeyConfig *key_config) : keyconfig(key_config) {}

    virtual void SetKeyConfiguration(KeyConfig *key_config) override {
        keyconfig = key_config;
    }

    virtual void ParseRaw(std::string const &input, Syllable &output) override {
        output = SylFromRaw(keyconfig, input);
    }

    virtual utf8_size_t RawToComposedCaret(Syllable &syllable, size_t raw_caret) override {
        return RawCaretToComposedCaret(keyconfig, syllable, raw_caret);
    }

    virtual size_t ComposedToRawCaret(Syllable &syllable, utf8_size_t composed_caret) override {
        return ComposedCaretToRawCaret(keyconfig, syllable, composed_caret);
    }

    virtual void ToFuzzy(const std::string &input, string_vector &output, bool &has_tone) {
        ComposedToRawWithAlternates(keyconfig, input, output, has_tone);
    }

    virtual std::vector<InputSequence> AsInputSequences(std::string const &word) override {
        auto ret = std::vector<InputSequence>();
        std::string word_copy = word;
        EraseAll(word_copy, kKhinDotStr);
        auto start = word_copy.cbegin();
        auto end = word_copy.cend();

        // 1 chunk = either 1 syllable or 1 separator
        auto chunks = std::vector<std::vector<std::string>>();
        auto sep = std::find_if(start, end, IsSyllableSeparator);
        auto has_tone = false;

        // Case 1: Only 1 syllable
        if (sep == end) {
            // Only one chunk = 1 syllable
            auto tmp = string_vector();
            ToFuzzy(word_copy, tmp, has_tone);
            for (auto &ea : tmp) {
                ret.push_back(InputSequence{ea, false});
            }
            if (has_tone) {
                tmp[0].pop_back();
                ret.push_back(InputSequence{std::move(tmp[0]), true});
            }
            return ret;
        }

        // Case 2: Multiple syllables
        auto push_chunk = [&](auto from, auto to) {
            auto syl = std::string(from, to);
            auto tmp = std::vector<std::string>();
            ToFuzzy(syl, tmp, has_tone);
            if (has_tone) {
                tmp.push_back(std::string(tmp[0].cbegin(), tmp[0].cend() - 1));
            }
            chunks.push_back(std::move(tmp));
            has_tone = false;
        };

        auto push_delim = [&](char ch) {
            chunks.push_back(std::vector<std::string>{std::string(1, ch), std::string()});
        };

        while (sep != end) {
            push_chunk(start, sep);
            // if (*sep != ' ') {
            //    push_delim(*sep);
            //}
            start = sep + 1;
            sep = std::find_if(start, end, IsSyllableSeparator);
        }
        // Get the last chunk
        push_chunk(start, sep);

        auto merged = odometer_merge(chunks);
        for (auto &ea : merged) {
            ret.push_back(InputSequence{std::move(ea), false});
        }
        return ret;
    }

    virtual TaiText AsBufferSegment(std::string const &raw, std::string const &target) override {
        auto ret = TaiText();
        auto t_start = target.cbegin();
        auto t_end = target.cend();
        auto r_start = raw.cbegin();
        auto r_end = raw.cend();

        auto sep = std::find_if(t_start, t_end, IsSyllableSeparator);

        while (sep != t_end) {
            auto syl = AlignRawToComposed(keyconfig, r_start, r_end, std::string(t_start, sep));
            ret.AddItem(syl);
            ret.AddItem(Spacer::VirtualSpace);
            t_start = sep + 1;
            sep = std::find_if(t_start, t_end, IsSyllableSeparator);
        }
        auto syl = AlignRawToComposed(keyconfig, r_start, r_end, std::string(t_start, sep));
        ret.AddItem(syl);

        return ret;
    }

    KeyConfig *keyconfig;
};

} // namespace

SyllableParser *SyllableParser::Create(KeyConfig *key_config) {
    return new SyllableParserImpl(key_config);
}

} // namespace khiin::engine

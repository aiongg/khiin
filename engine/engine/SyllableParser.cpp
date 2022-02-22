#include "SyllableParser.h"

#include <algorithm>
#include <iterator>

#include "KeyConfig.h"
#include "Lomaji.h"
#include "Syllable.h"
#include "TaiText.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {
using namespace unicode;
namespace u8u = utf8::unchecked;

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

inline constexpr char *kOrderedToneablesIndex2[] = {"oa", "oe"};
inline constexpr char *kOrderedToneablesIndex1[] = {"o", "a", "e", "u", "i", "ng", "m"};
inline constexpr char kToneableLetters[] = {'a', 'e', 'i', 'm', 'n', 'o', 'u'};

inline constexpr char kSyllableSeparator[] = {' ', '-'};

const std::unordered_map<Tone, std::string> kToneToUnicodeMap = {
    {Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"}, {Tone::T5, u8"\u0302"}, {Tone::T7, u8"\u0304"},
    {Tone::T8, u8"\u030d"}, {Tone::T9, u8"\u0306"}, {Tone::TK, u8"\u00b7"}};

const std::string kKhinDotStr = u8"\u00b7";
const std::string kSpaceStr = u8" ";

inline bool IsSyllableSeparator(char ch) {
    return ch == '-' || ch == ' ';
}

inline bool EndsWithPtkh(std::string_view str) {
    auto ch = str.back();
    return ch == 'p' || ch == 't' || ch == 'k' || ch == 'h';
}

bool HasToneable(std::string_view str) {
    for (auto &c1 : str) {
        for (auto &c2 : kToneableLetters) {
            if (tolower(c1) == c2) {
                return true;
            }
        }
    }

    return false;
}

size_t FindTonePosition(std::string_view strv) {
    auto str = unicode::copy_str_tolower(strv);

    for (auto &sequence : kOrderedToneablesIndex2) {
        if (auto i = str.find(sequence); i != std::string::npos && str.size() > i + 2) {
            return i + 2;
        }
    }

    for (auto &letter : kOrderedToneablesIndex1) {
        if (auto i = str.find(letter); i != std::string::npos) {
            return i + 1;
        }
    }

    return std::string::npos;
}

void ParseRawKhin(KeyConfig *keyconfig, Syllable &syl) {
    if (syl.raw_body.empty()) {
        return;
    }

    if (syl.raw_body.size() > 1) {
        auto hyphen_keys = keyconfig->GetHyphenKeys();
        for (auto key : hyphen_keys) {
            if (syl.raw_body[0] == key && syl.raw_body[1] == key) {
                syl.raw_body.erase(0, 2);
                syl.khin_pos = KhinKeyPosition::Start;
                syl.khin_key = key;
                return;
            }
        }
    }

    auto khin_keys = keyconfig->GetKhinKeys();
    for (auto key : khin_keys) {
        if (syl.raw_body.back() == key) {
            syl.raw_body.pop_back();
            syl.khin_pos = KhinKeyPosition::End;
            syl.khin_key = key;
            return;
        }
    }
}

bool RemoveToneChar(KeyConfig *keyconfig, Syllable &syl) {
    if (!HasToneable(syl.raw_body)) {
        return false;
    }

    syl.tone = keyconfig->CheckToneKey(syl.raw_body.back());
    if (syl.tone == Tone::NaT) {
        return false;
    }

    syl.tone_key = syl.raw_body.back();
    syl.raw_body.pop_back();
    return true;
}

void RemoveToneDiacritic(std::string &input) {
    auto nfd = unicode::to_nfd(input);
    for (auto &[t, t_str] : kToneToUnicodeMap) {
        if (auto i = input.find(t_str); i != std::string::npos) {
            input.erase(i, t_str.size());
            return;
        }
    }
}

bool HasToneDiacritic(std::string const &str) {
    auto s = unicode::to_nfd(str);
    auto it = s.cbegin();
    while (it != s.cend()) {
        auto cp = u8u::next(it);
        if (is_tone(cp)) {
            return true;
        }
    }
    return false;
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

void ApplyConversionRules(KeyConfig *keyconfig, std::string &output) {
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
    if (tone == Tone::NaT) {
        return false;
    }

    size_t tone_index = FindTonePosition(toneless);
    if (tone_index == std::string::npos) {
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
    ParseRawKhin(keyconfig, output);
    RemoveToneChar(keyconfig, output);
    output.composed = output.raw_body;
    ApplyConversionRules(keyconfig, output.composed);
    if (Lomaji::NeedsToneDiacritic(output.tone)) {
        AddToneDiacritic(output.tone, output.composed);
    }
    if (output.khin_pos != KhinKeyPosition::None) {
        output.composed.insert(0, kKhinDotStr);
    }
    output.composed = unicode::to_nfc(output.composed);
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

Syllable SylFromComposed(KeyConfig *keyconfig, std::string const &input, char original_tone_key = 0) {
    auto copy = to_nfd(input);
    auto tone = Tone::NaT;
    char digit_key = 0;
    char telex_key = 0;
    RemoveToneDiacriticSaveTone(keyconfig, copy, tone, digit_key, telex_key);
    ApplyDeconversionRules(keyconfig, copy);
    auto syl = Syllable();
    syl.composed = input;
    syl.raw_body = copy;
    syl.tone = tone;
    if (tone != Tone::NaT && original_tone_key) {
        syl.tone_key = original_tone_key;
    } else {
        syl.tone_key = digit_key;
    }
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

    while (r_it != r_end && t_it != t_end && tolower(*r_it) == tolower(*t_it)) {
        ++r_it;
        ++t_it;
    }

    if (r_it != r_end && compare.tone != Tone::NaT) {
        auto r_maybe_tone = keyconfig->CheckToneKey(r_it[-1]);
        if (r_maybe_tone != compare.tone) {
            r_maybe_tone = keyconfig->CheckToneKey(r_it[0]);
            if (r_maybe_tone == compare.tone) {
                ++r_it;
            }
        }
    }

    auto r_syl = std::string(r_begin, r_it);
    r_begin = r_it;
    return SylFromRaw(keyconfig, r_syl);
}

void ComposedToRawWithAlternates(KeyConfig *keyconfig, const std::string &input, std::vector<std::string> &output,
                                 bool &has_tone) {
    auto syl = Lomaji::Decompose(input);
    auto tone = Tone::NaT;
    char digit_key = 0;
    char telex_key = 0;
    auto found_tone = RemoveToneDiacriticSaveTone(keyconfig, syl, tone, digit_key, telex_key);
    ApplyDeconversionRules(keyconfig, syl);
    unicode::str_tolower(syl);

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

utf8_size_t RawCaretToComposedCaret(KeyConfig *keyconfig, Syllable const &syllable, size_t raw_caret) {
    auto const &input = syllable.raw_input;
    auto const &body = syllable.raw_body;
    auto ret = std::string::npos;

    if (raw_caret == input.size()) {
        ret = u8_size(syllable.composed);
    } else if (raw_caret < input.size()) {
        auto lhs = std::string(body.cbegin(), body.cbegin() + raw_caret);
        ApplyConversionRules(keyconfig, lhs);

        if (syllable.tone != Tone::NaT) {
            size_t tone_pos = FindTonePosition(body);
            if (tone_pos <= lhs.size()) {
                AddToneDiacritic(syllable.tone, lhs);
            }
        }
        ret = u8_size(to_nfc(lhs));
    }

    return ret;
}

size_t ComposedCaretToRawCaret(KeyConfig *keyconfig, Syllable const &syllable, utf8_size_t composed_caret) {
    size_t size = utf8::distance(syllable.composed.cbegin(), syllable.composed.cend());
    auto ret = std::string::npos;

    if (composed_caret == size) {
        ret = syllable.raw_input.size();
    } else if (composed_caret < size) {
        auto caret = syllable.composed.cbegin();
        u8u::advance(caret, composed_caret);
        auto lhs = to_nfd(std::string(syllable.composed.cbegin(), caret));

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

    virtual utf8_size_t RawToComposedCaret(Syllable const &syllable, size_t raw_caret) override {
        return RawCaretToComposedCaret(keyconfig, syllable, raw_caret);
    }

    virtual size_t ComposedToRawCaret(Syllable const &syllable, utf8_size_t composed_caret) override {
        return ComposedCaretToRawCaret(keyconfig, syllable, composed_caret);
    }

    virtual void ToFuzzy(std::string const &input, std::vector<std::string> &output, bool &has_tone) {
        ComposedToRawWithAlternates(keyconfig, input, output, has_tone);
    }

    virtual std::vector<InputSequence> AsInputSequences(std::string const &word) override {
        if (word == u8"iăn-jín") {
            auto x = 3;
        }
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
            auto tmp = std::vector<std::string>();
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

    virtual TaiText AsTaiText(std::string const &raw, std::string const &target) override {
        auto ret = TaiText();
        auto raw_lc = copy_str_tolower(raw);
        auto t_start = target.cbegin();
        auto t_end = target.cend();
        auto r_start = raw.cbegin();
        auto r_end = raw.cend();

        auto sep = std::find_if(t_start, t_end, IsSyllableSeparator);

        while (sep != t_end) {
            auto syl = AlignRawToComposed(keyconfig, r_start, r_end, std::string(t_start, sep));
            ret.AddItem(syl);

            if (r_start == r_end) {
                return ret;
            }

            ret.AddItem(VirtualSpace());
            t_start = sep + 1;
            sep = std::find_if(t_start, t_end, IsSyllableSeparator);
        }
        auto syl = AlignRawToComposed(keyconfig, r_start, r_end, std::string(t_start, sep));
        ret.AddItem(syl);

        return ret;
    }

    virtual void Erase(Syllable &syllable, utf8_size_t index) override {
        auto size = u8_size(syllable.composed);
        if (index > size) {
            return;
        }

        auto from = syllable.composed.begin();
        u8u::advance(from, index);

        auto curs_end = Lomaji::MoveCaret(syllable.composed, index, CursorDirection::R);
        auto to = syllable.composed.begin();
        u8u::advance(to, curs_end);

        if (HasToneDiacritic(std::string(from, to))) {
            syllable.tone = Tone::NaT;
            syllable.tone_key = 0;
        }

        syllable.composed.erase(from, to);
        syllable = SylFromComposed(keyconfig, syllable.composed, syllable.tone_key);
    }

    virtual void SetKhin(Syllable &syllable, KhinKeyPosition khin_pos, char khin_key) override {
        if (syllable.khin_pos == KhinKeyPosition::None && khin_pos != KhinKeyPosition::None) {
            if (khin_pos != KhinKeyPosition::Virtual) {
                syllable.raw_input.insert(0, 2, khin_key);
            }
            syllable.composed.insert(0, kKhinDotStr);
        } else if (syllable.khin_pos != KhinKeyPosition::None && khin_pos == KhinKeyPosition::None &&
                   syllable.composed.find(kKhinDotStr) == 0) {
            if (syllable.khin_pos != KhinKeyPosition::Virtual) {
                syllable.raw_input.erase(0, 2);
            }
            syllable.composed.erase(0, kKhinDotStr.size());
        }

        syllable.khin_pos = khin_pos;
        syllable.khin_key = khin_key;
    }

  private:
    KeyConfig *keyconfig;
};

} // namespace

SyllableParser *SyllableParser::Create(KeyConfig *key_config) {
    return new SyllableParserImpl(key_config);
}

} // namespace khiin::engine

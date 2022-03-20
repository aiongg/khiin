#include "SyllableParser.h"

#include <algorithm>
#include <iterator>

#include "config/Config.h"
#include "config/KeyConfig.h"
#include "utils/unicode.h"

#include "Engine.h"
#include "Lomaji.h"
#include "Syllable.h"
#include "TaiText.h"

namespace khiin::engine {
namespace {
using namespace unicode;
namespace u8u = utf8::unchecked;

// Merges sets of strings using an "odometer-like" wrapping iterator
template <typename T>
std::vector<T> odometer_merge(std::vector<std::vector<T>> const &vector_set) {
    auto size = vector_set.size();
    auto ret = std::vector<T>();
    auto its = std::vector<typename std::vector<T>::const_iterator>();
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

inline constexpr std::array<char *, 7> kOrderedToneablesIndex1 = {{"o", "a", "e", "u", "i", "ng", "m"}};
inline constexpr std::array<char *, 3> kOrderedToneablesIndex2 = {{"oa", "oe"}};
inline constexpr std::array<char, 8> kToneableLetters = {'a', 'e', 'i', 'm', 'n', 'o', 'u'};

inline constexpr std::array<char, 2> kSyllableSeparator = {' ', '-'};

const std::unordered_map<Tone, std::string> kToneToUnicodeMap = {{Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"},
                                                                 {Tone::T5, u8"\u0302"}, {Tone::T7, u8"\u0304"},
                                                                 {Tone::T8, u8"\u030d"}, {Tone::T9, u8"\u0306"}};

const std::string kKhinDotStr = u8"\u00b7";
const std::string kKhinHyphenStr = "--";
const std::string kSpaceStr = u8" ";

inline bool IsSyllableSeparator(char ch) {
    return ch == '-' || ch == ' ';
}

inline bool EndsWithPtkh(std::string_view str) {
    auto ch = str.back();
    return ch == 'p' || ch == 't' || ch == 'k' || ch == 'h';
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

void EraseAll(std::string &input, std::string const &substr) {
    auto idx = std::string::npos;
    while ((idx = input.find(substr)) != std::string::npos) {
        input.erase(idx, substr.size());
    }
}

void ComposedToRawWithAlternates(KeyConfig *keyconfig, const std::string &input, std::vector<std::string> &output,
                                 bool &has_tone) {
    auto syl = Lomaji::Decompose(input);
    auto tone = Tone::NaT;
    char digit_key = 0;
    char telex_key = 0;
    auto found_tone = RemoveToneDiacriticSaveTone(keyconfig, syl, tone, digit_key, telex_key);
    syl = keyconfig->Deconvert(syl);
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

class SyllableParserImpl : public SyllableParser {
  public:
    SyllableParserImpl(Engine *engine) : m_engine(engine) {}

    bool DottedKhin() {
        return m_engine->config()->dotted_khin();
    }

    Syllable ParseRaw(std::string const &input) override {
        auto syl = Syllable(m_engine->keyconfig(), DottedKhin());
        syl.SetRawInput(input);
        return syl;
    }

    Syllable ParseComposed(std::string const &input) override {
        auto syl = Syllable(m_engine->keyconfig(), DottedKhin());
        syl.SetComposed(input);
        return syl;
    }

    void ToFuzzy(std::string const &input, std::vector<std::string> &output, bool &has_tone) {
        ComposedToRawWithAlternates(m_engine->keyconfig(), input, output, has_tone);
    }

    std::vector<InputSequence> AsInputSequences(std::string const &word) override {
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

    TaiText AsTaiText(std::string const &raw, std::string const &target) override {
        auto ret = TaiText();
        auto t_start = target.cbegin();
        auto t_end = target.cend();
        auto r_start = raw.cbegin();
        auto r_end = raw.cend();

        auto sep = std::find_if(t_start, t_end, IsSyllableSeparator);

        auto targ_syl = [&]() {
            return ParseComposed(std::string(t_start, sep));
        };

        while (sep != t_end) {
            auto syl = AlignRawToComposed(targ_syl(), r_start, r_end);
            ret.AddItem(syl);

            if (r_start == r_end) {
                return ret;
            }

            ret.AddItem(VirtualSpace());
            t_start = sep + 1;
            sep = std::find_if(t_start, t_end, IsSyllableSeparator);
        }
        auto syl = AlignRawToComposed(targ_syl(), r_start, r_end);
        ret.AddItem(syl);

        return ret;
    }

  private:
    // |raw| may be more than one syllable, and may have separators or tones or not.
    // |target| is exactly one syllable.
    Syllable AlignRawToComposed(Syllable const &target, std::string::const_iterator &r_begin,
                                std::string::const_iterator const &r_end) {

        auto const &target_raw = target.raw_input();
        auto target_tone = target.tone();
        auto keyconfig = m_engine->keyconfig();
        auto r_it = r_begin;
        auto t_it = target_raw.cbegin();
        auto t_end = target_raw.cend();

        while (r_it != r_end && t_it != t_end && tolower(*r_it) == tolower(*t_it)) {
            ++r_it;
            ++t_it;
        }

        if (r_it != r_end && target_tone != Tone::NaT) {
            auto r_maybe_tone = keyconfig->CheckToneKey(r_it[-1]);
            if (r_maybe_tone != target_tone) {
                r_maybe_tone = keyconfig->CheckToneKey(r_it[0]);
                if (r_maybe_tone == target_tone) {
                    ++r_it;
                }
            }
        } else if (r_it != r_end) {
            auto r_maybe_tone = keyconfig->CheckToneKey(r_it[0]);
            if (r_maybe_tone == Tone::T1 || r_maybe_tone == Tone::T4) {
                ++r_it;
            }
        }

        auto r_syl = std::string(r_begin, r_it);
        r_begin = r_it;
        return ParseRaw(r_syl);
    }

    Engine *m_engine;
};

} // namespace

std::unique_ptr<SyllableParser> SyllableParser::Create(Engine *engine) {
    return std::make_unique<SyllableParserImpl>(engine);
}

} // namespace khiin::engine

#include "Transliterator.h"

#include <assert.h>
#include <unordered_map>

#include "Syllable.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {

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

// inline const std::string U8_UR = u8"\u1e73";
// inline const std::string U8_UR_UC = u8"\u1e72";

static inline const std::unordered_map<SpecialGlyph, char32_t> kSpecialGlyphUnicodeMap = {
    {SpecialGlyph::Nasal, kNasalLower},
    {SpecialGlyph::NasalUpper, kNasalUpper},
    {SpecialGlyph::DotAboveRight, kDotAboveRight},
    {SpecialGlyph::DotsBelow, kDotsBelow},
    {SpecialGlyph::KhinDot, kKhinDot}};

static inline const std::unordered_map<Tone, std::string> kToneToUnicodeMap = {
    {Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"}, {Tone::T5, u8"\u0302"},
    {Tone::T7, u8"\u0304"}, {Tone::T8, u8"\u030d"}, {Tone::T9, u8"\u0306"}};

static inline const std::string kKhinDotStr = u8"\u00b7";

static inline const std::unordered_map<Tone, char> kToneDigitMap = {{Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'},
                                                                    {Tone::T7, '7'}, {Tone::T8, '8'}, {Tone::T9, '9'}};

struct ConversionRule {
    std::string input;
    std::string output;
};

struct ToneKey {
    Tone tone;
    char key;
};

const std::vector<std::string> kOrderedToneableCombinations = {"OA", "Oa", "oa", "OE", "Oe", "oe"};
const std::vector<char> kOrderedToneableLetters = {'O', 'o', 'A', 'a', 'E', 'e', 'U',
                                                   'u', 'I', 'i', 'M', 'm', 'N', 'n'};

int FindToneableIndex(std::string const &unicode) {
    for (auto &combo : kOrderedToneableCombinations) {
        if (auto i = unicode.find(combo); i != std::string::npos) {
            if (unicode.size() - i > 3) {
                return i + 2;
            }
        }
    }

    for (auto &letter : kOrderedToneableLetters) {
        if (auto i = unicode.find(letter); i != std::string::npos) {
            return i + 1;
        }
    }
}

class TransliteratorImpl : public Transliterator {
  public:
    virtual std::string Precompose(std::string const &ascii, ToneInfo const &tone_info) override {
        auto input = std::string(ascii);

        // SeparateFinalToneKey(input, tone);

        auto ret = std::string();
        auto it = input.cbegin();
        auto cend = input.cend();
    outer_loop:
        while (it != cend) {
            for (auto &rule : conversion_rules) {
                auto remaining_size = std::distance(it, cend);
                auto rule_input_size = rule.input.size();
                if (remaining_size >= rule_input_size &&
                    std::equal(it, it + rule_input_size, rule.input.cbegin(), rule.input.cend())) {
                    ret.append(rule.output.cbegin(), rule.output.cend());
                    it += rule.input.size();
                    goto outer_loop;
                }
            }
            ret.push_back(it[0]);
            ++it;
        }

        if (auto tone = tone_info.tone; tone != Tone::NaT) {
            auto i = FindToneableIndex(ret);
            ret.insert(i, kToneToUnicodeMap.at(tone));
        }

        if (tone_info.khin_pos != KhinKeyPosition::None) {
            ret.insert(0, kKhinDotStr);
        }

        return toNFC(ret);
    }

    virtual std::string Decompose(std::string const &unicode, ToneInfo const &tone_info) override {
        auto input = toNFD(unicode);
        auto no_tone = Tone::NaT;
        SeparateToneGlyph(input, no_tone);

        auto ret = std::string();
        auto it = input.cbegin();
        auto cend = input.cend();
    outer_loop:
        while (it != cend) {
            for (auto &rule : conversion_rules) {
                auto remaining_size = std::distance(it, cend);
                auto rule_output_size = rule.output.size();
                if (remaining_size >= rule_output_size &&
                    std::equal(it, it + rule_output_size, rule.output.cbegin(), rule.output.cend())) {
                    ret.append(rule.input.cbegin(), rule.input.cend());
                    it += rule.output.size();
                    goto outer_loop;
                }
            }
            ret.push_back(it[0]);
            ++it;
        }

        if (auto tone = tone_info.tone; tone != Tone::NaT) {
            ret.push_back(tone_info.tone_key);
        }

        if (auto khin = tone_info.khin_pos; khin != KhinKeyPosition::None) {
            if (khin == KhinKeyPosition::Start) {
                ret.insert(0, 2, tone_info.khin_key);
            } else if (khin == KhinKeyPosition::End) {
                ret.push_back(tone_info.khin_key);
            }
        }

        return ret;
    }

    virtual void AddConversionRule(std::string input, std::string output) override {
        conversion_rules.push_back(ConversionRule{input, output});
    }

    virtual void AddToneKey(Tone tone, char key) override {
        tone_keys.push_back(ToneKey{tone, key});
    }

    //virtual void InsertAt(std::string input, int at_in, char char_in, std::string &output, int &at_out) override {
    //    if (input.empty()) {
    //        output = Precompose(std::string(1, char_in), ToneInfo{});
    //        at_out = output.size();
    //        return;
    //    }

    //    auto it = input.cbegin();
    //    for (auto i = 0; i < at_in; ++i) {
    //        utf8::unchecked::advance(it, 1);
    //    }

    //    auto lhs = std::string(input.cbegin(), it);
    //    auto rhs = std::string(it, input.cend());
    //    auto lhs_decomp = Decompose(lhs, ToneInfo{});
    //    auto rhs_decomp = Decompose(rhs, ToneInfo{});

    //    if (lhs_decomp.size() > 1) {
    //        char maybe_tone_key = lhs_decomp.back();
    //        for (auto &tone_key : tone_keys) {
    //            if (maybe_tone_key == tone_key.key) {
    //                lhs_decomp.pop_back();
    //                rhs_decomp.push_back(maybe_tone_key);
    //                break;
    //            }
    //        }
    //    }

    //    lhs_decomp.push_back(char_in);
    //    lhs = Precompose(lhs_decomp, ToneInfo{});
    //    at_out = Utf8Size(lhs);
    //    output = Precompose(lhs_decomp + rhs_decomp, ToneInfo{});
    //}

    virtual size_t DecomposedSize(std::string const &composed, utf8_size_t to) override {
        auto it = composed.cbegin();
        for (auto i = 0; i < to; ++i) {
            utf8::unchecked::advance(it, 1);
        }

        if (it == composed.cend()) {
            return utf8::distance(composed.cbegin(), it);
        }

        auto lhs = std::string(composed.cbegin(), it);
        auto lhs_decomp = Decompose(lhs, ToneInfo{});

        if (lhs_decomp.size() > 1) {
            char maybe_tone_key = lhs_decomp.back();
            for (auto &tone_key : tone_keys) {
                if (maybe_tone_key == tone_key.key) {
                    lhs_decomp.pop_back();
                    break;
                }
            }
        }

        return lhs_decomp.size();
    };

  private:
    void SeparateFinalToneKey(std::string &input, Tone &tone) {
        auto maybe_tone_key = input.back();

        for (auto &tone_key : tone_keys) {
            if (maybe_tone_key == tone_key.key) {
                input.pop_back();
                tone = tone_key.tone;
                return;
            }
        }
    }

    void SeparateToneGlyph(std::string &input, Tone &tone) {
        for (auto &[t, str] : kToneToUnicodeMap) {
            if (auto idx = input.find(str); idx != std::string::npos) {
                input.erase(idx, str.size());
                tone = t;
                return;
            }
        }
    }

    std::vector<ConversionRule> conversion_rules;
    std::vector<ToneKey> tone_keys;
};

Transliterator *default_transliterator = nullptr;

} // namespace

Transliterator *Transliterator::Create() {
    return new TransliteratorImpl();
}

Transliterator *Transliterator::default_instance() {
    if (default_transliterator) {
        return default_transliterator;
    }

    auto t = new TransliteratorImpl();

    t->AddConversionRule("nn", u8"\u207f");
    t->AddConversionRule("ou", u8"o\u0358");
    t->AddConversionRule("or", u8"o\u0324");
    t->AddConversionRule("ur", u8"u\u0324");

    t->AddConversionRule("NN", u8"\u1d3a");
    t->AddConversionRule("OU", u8"O\u0358");
    t->AddConversionRule("OR", u8"O\u0324");
    t->AddConversionRule("UR", u8"U\u0324");

    t->AddConversionRule("nN", u8"\u207f");
    t->AddConversionRule("oU", u8"o\u0358");
    t->AddConversionRule("oR", u8"o\u0324");
    t->AddConversionRule("uR", u8"u\u0324");

    t->AddConversionRule("Nn", u8"\u1d3a");
    t->AddConversionRule("Ou", u8"O\u0358");
    t->AddConversionRule("Or", u8"O\u0324");
    t->AddConversionRule("Ur", u8"U\u0324");

    t->AddToneKey(Tone::T2, '2');
    t->AddToneKey(Tone::T3, '3');
    t->AddToneKey(Tone::T5, '5');
    t->AddToneKey(Tone::T7, '7');
    t->AddToneKey(Tone::T8, '8');
    t->AddToneKey(Tone::T9, '9');

    default_transliterator = t;
    return default_transliterator;
}

} // namespace khiin::engine

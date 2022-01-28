#include "Transliterator.h"

#include <assert.h>
#include <unordered_map>

#include <unilib/uninorms.h>
#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

namespace khiin::engine {
namespace {

static std::string toNFD(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);
    return std::move(utf8::utf32to8(u32s));
}

static std::string toNFC(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfc(u32s);
    return std::move(utf8::utf32to8(u32s));
}

static inline int utf8Size(std::string s) {
    return static_cast<int>(utf8::distance(s.begin(), s.end()));
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
    virtual std::string Precompose(std::string const &ascii) override {
        auto input = std::string(ascii);
        auto tone = Tone::NaT;

        SeparateFinalToneKey(input, tone);

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

        if (tone != Tone::NaT) {
            auto i = FindToneableIndex(ret);
            ret.insert(i, kToneToUnicodeMap.at(tone));
        }

        return toNFC(ret);
    }

    virtual std::string Decompose(std::string const &unicode) override {
        auto input = toNFD(unicode);
        auto tone = Tone::NaT;

        SeparateToneGlyph(input, tone);

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

        if (tone != Tone::NaT) {
            ret.push_back(kToneDigitMap.at(tone));
        }

        return ret;
    }

    virtual void AddConversionRule(std::string input, std::string output) override {
        conversion_rules.push_back(ConversionRule{input, output});
    }

    virtual void AddToneKey(Tone tone, char key) override {
        tone_keys.push_back(ToneKey{tone, key});
    }

    virtual void InsertAt(std::string &composed, int &index, char char_in) override {
        if (composed.empty()) {
            composed = Precompose(std::string(1, char_in));
            index = composed.size();
            return;
        }

        auto it = composed.cbegin();
        for (auto i = 0; i < index; ++i) {
            utf8::unchecked::advance(it, 1);
        }

        auto lhs = std::string(composed.cbegin(), it);
        auto rhs = std::string(it, composed.cend());
        auto lhs_decomp = Decompose(lhs);
        auto rhs_decomp = Decompose(rhs);

        if (lhs_decomp.size() > 1) {
            char maybe_tone_key = lhs_decomp.back();
            for (auto &tone_key : tone_keys) {
                if (maybe_tone_key == tone_key.key) {
                    lhs_decomp.pop_back();
                    rhs_decomp.push_back(maybe_tone_key);
                    break;
                }
            }
        }

        lhs_decomp.push_back(char_in);
        lhs = Precompose(lhs_decomp);
        index = utf8Size(lhs);
        composed = Precompose(lhs_decomp + rhs_decomp);
    }

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

} // namespace

Transliterator *Transliterator::Create() {
    return new TransliteratorImpl();
}

} // namespace khiin::engine

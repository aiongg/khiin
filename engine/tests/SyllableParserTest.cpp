#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "KeyConfig.h"
#include "Syllable.h"
#include "SyllableParser.h"
#include "TaiText.h"

namespace khiin::engine {
namespace {

using ::testing::Contains;

std::vector<std::string> input_sequence_strings_only(std::vector<InputSequence> const &inputs) {
    auto ret = std::vector<std::string>();
    for (auto const &ea : inputs) {
        ret.push_back(ea.input);
    }
    return ret;
}

struct SyllableParserTest : ::testing::Test {
  protected:
    void SetUp() {
        keyconfig = KeyConfig::Create();
        parser = SyllableParser::Create(keyconfig);
    }
    KeyConfig *keyconfig = nullptr;
    SyllableParser *parser = nullptr;
};

TEST_F(SyllableParserTest, ParseRaw_ho2) {
    auto input = "ho2";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "ho");
    EXPECT_EQ(syl.tone, Tone::T2);
    EXPECT_EQ(syl.tone_key, '2');
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::None);
    EXPECT_EQ(syl.composed, u8"hó");
    EXPECT_EQ(syl.raw_body.size(), 2);
}

TEST_F(SyllableParserTest, ParseRaw_hou7) {
    auto input = "hou7";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "hou");
    EXPECT_EQ(syl.tone, Tone::T7);
    EXPECT_EQ(syl.tone_key, '7');
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::None);
    EXPECT_EQ(syl.composed, u8"hō\u0358");
}

TEST_F(SyllableParserTest, ParseRaw_an1) {
    auto input = "an1";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "an");
    EXPECT_EQ(syl.tone, Tone::T1);
    EXPECT_EQ(syl.tone_key, '1');
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::None);
    EXPECT_EQ(syl.composed, u8"an");
}

TEST_F(SyllableParserTest, ParseRaw_bah4) {
    auto input = "bah4";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "bah");
    EXPECT_EQ(syl.tone, Tone::T4);
    EXPECT_EQ(syl.tone_key, '4');
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::None);
    EXPECT_EQ(syl.composed, u8"bah");
}

TEST_F(SyllableParserTest, ParseRaw_ouhnn8) {
    auto input = "ouhnn8";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "ouhnn");
    EXPECT_EQ(syl.tone, Tone::T8);
    EXPECT_EQ(syl.tone_key, '8');
    EXPECT_EQ(syl.composed, u8"o\u030d\u0358h\u207f");
}

TEST_F(SyllableParserTest, ParseRaw_ian9) {
    auto input = "ian9";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "ian");
    EXPECT_EQ(syl.tone, Tone::T9);
    EXPECT_EQ(syl.tone_key, '9');
    EXPECT_EQ(syl.composed, u8"iăn");
}

TEST_F(SyllableParserTest, ParseRaw_mng5) {
    auto input = "mng5";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "mng");
    EXPECT_EQ(syl.tone, Tone::T5);
    EXPECT_EQ(syl.tone_key, '5');
    EXPECT_EQ(syl.composed, u8"mn\u0302g");
}

TEST_F(SyllableParserTest, ParseRaw_hur5) {
    auto input = "hur5";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "hur");
    EXPECT_EQ(syl.tone, Tone::T5);
    EXPECT_EQ(syl.tone_key, '5');
    EXPECT_EQ(syl.composed, u8"hṳ\u0302");
}

TEST_F(SyllableParserTest, ParseRaw_hor3) {
    auto input = "hor3";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "hor");
    EXPECT_EQ(syl.tone, Tone::T3);
    EXPECT_EQ(syl.tone_key, '3');
    EXPECT_EQ(syl.composed, u8"hò\u0324");
}

TEST_F(SyllableParserTest, ParseRaw_khin_only) {
    auto input = "--";
    auto syl = Syllable();

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "");
    EXPECT_EQ(syl.tone, Tone::NaT);
    EXPECT_EQ(syl.tone_key, 0);
    EXPECT_EQ(syl.composed, u8"\u00b7");
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::Start);
    EXPECT_EQ(syl.khin_key, '-');
}

TEST_F(SyllableParserTest, ConvertCaretPosition) {
    auto syl = Syllable();
    syl.raw_input = "ho2";
    syl.raw_body = "ho";
    syl.tone = Tone::T2;
    syl.tone_key = '2';
    syl.composed = u8"hó";
    size_t caret = std::string::npos;

    caret = parser->RawToComposedCaret(syl, 0);
    EXPECT_EQ(caret, 0);
    caret = parser->RawToComposedCaret(syl, 1);
    EXPECT_EQ(caret, 1);
    caret = parser->RawToComposedCaret(syl, 3);
    EXPECT_EQ(caret, 2);
    caret = parser->RawToComposedCaret(syl, 4);
    EXPECT_EQ(caret, std::string::npos);

    caret = parser->ComposedToRawCaret(syl, 0);
    EXPECT_EQ(caret, 0);
    caret = parser->ComposedToRawCaret(syl, 1);
    EXPECT_EQ(caret, 1);
    caret = parser->ComposedToRawCaret(syl, 2);
    EXPECT_EQ(caret, 3);
    caret = parser->ComposedToRawCaret(syl, 3);
    EXPECT_EQ(caret, std::string::npos);
}

TEST_F(SyllableParserTest, ConvertCaretPosition2) {
    auto syl = Syllable();
    syl.raw_input = "hounn3";
    syl.raw_body = "hounn";
    syl.tone = Tone::T3;
    syl.tone_key = '3';
    syl.composed = u8"hò\u0358ⁿ";
    size_t caret = std::string::npos;

    caret = parser->RawToComposedCaret(syl, 0);
    EXPECT_EQ(caret, 0);
    caret = parser->RawToComposedCaret(syl, 1);
    EXPECT_EQ(caret, 1);
    caret = parser->RawToComposedCaret(syl, 3);
    EXPECT_EQ(caret, 3);
    caret = parser->RawToComposedCaret(syl, 6);
    EXPECT_EQ(caret, 4);
    caret = parser->RawToComposedCaret(syl, 7);
    EXPECT_EQ(caret, std::string::npos);

    caret = parser->ComposedToRawCaret(syl, 0);
    EXPECT_EQ(caret, 0);
    caret = parser->ComposedToRawCaret(syl, 1);
    EXPECT_EQ(caret, 1);
    caret = parser->ComposedToRawCaret(syl, 3);
    EXPECT_EQ(caret, 3);
    caret = parser->ComposedToRawCaret(syl, 4);
    EXPECT_EQ(caret, 6);
    caret = parser->ComposedToRawCaret(syl, 5);
    EXPECT_EQ(caret, std::string::npos);
}

TEST_F(SyllableParserTest, ToFuzzy) {
    auto input = u8"hō\u0358";
    auto result = std::vector<std::string>();
    auto has_tone = false;
    parser->ToFuzzy(input, result, has_tone);

    EXPECT_THAT(result, Contains("hou7"));
    EXPECT_TRUE(has_tone);
}

TEST_F(SyllableParserTest, AsInputs_pengan) {
    auto input = u8"pêng-an";
    auto result = input_sequence_strings_only(parser->AsInputSequences(input));

    EXPECT_EQ(result.size(), 4);
    EXPECT_THAT(result, Contains(u8"pengan"));
    EXPECT_THAT(result, Contains(u8"peng5an"));
    EXPECT_THAT(result, Contains(u8"pengan1"));
    EXPECT_THAT(result, Contains(u8"peng5an1"));
}

TEST_F(SyllableParserTest, AsInputs_lohkhi) {
    auto input = u8"lo\u030dh-·khì";
    auto result = input_sequence_strings_only(parser->AsInputSequences(input));

    EXPECT_EQ(result.size(), 4);
    EXPECT_THAT(result, Contains(u8"lohkhi"));
    EXPECT_THAT(result, Contains(u8"lohkhi3"));
    EXPECT_THAT(result, Contains(u8"loh8khi"));
    EXPECT_THAT(result, Contains(u8"loh8khi3"));
}

TEST_F(SyllableParserTest, AsInputs_peng5) {
    auto input = u8"pêng";
    auto result = parser->AsInputSequences(input);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].input, "peng5");
    EXPECT_FALSE(result[0].is_fuzzy_monosyllable);
    EXPECT_EQ(result[1].input, "peng");
    EXPECT_TRUE(result[1].is_fuzzy_monosyllable);
}

TEST_F(SyllableParserTest, AsInputs_abichok) {
    auto input = u8"a-bí-cho\u030dk";
    auto result = input_sequence_strings_only(parser->AsInputSequences(input));

    EXPECT_EQ(result.size(), 8);
    EXPECT_THAT(result, Contains(u8"a1bi2chok8"));
    EXPECT_THAT(result, Contains(u8"abi2chok8"));
    EXPECT_THAT(result, Contains(u8"a1bi2chok"));
    EXPECT_THAT(result, Contains(u8"a1bichok8"));
    EXPECT_THAT(result, Contains(u8"abichok8"));
    EXPECT_THAT(result, Contains(u8"abi2chok"));
    EXPECT_THAT(result, Contains(u8"a1bichok"));
    EXPECT_THAT(result, Contains(u8"abichok"));
}

TEST_F(SyllableParserTest, AsInputs_an) {
    auto input = u8"an";
    auto result = parser->AsInputSequences(input);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].input, "an");
    EXPECT_FALSE(result[0].is_fuzzy_monosyllable);
    EXPECT_EQ(result[1].input, "an1");
    EXPECT_FALSE(result[1].is_fuzzy_monosyllable);
}

TEST_F(SyllableParserTest, AsInputs_bah) {
    auto input = u8"bah";
    auto result = parser->AsInputSequences(input);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].input, "bah");
    EXPECT_FALSE(result[0].is_fuzzy_monosyllable);
    EXPECT_EQ(result[1].input, "bah4");
    EXPECT_FALSE(result[1].is_fuzzy_monosyllable);
}

TEST_F(SyllableParserTest, TaiText_pengan) {
    auto raw = "pengan";
    auto target = u8"pêng-an";

    auto result = parser->AsTaiText(raw, target);
    EXPECT_EQ(result.raw(), "pengan");
    EXPECT_EQ(result.composed(), "peng an");
    EXPECT_EQ(result.size(), 7);
}

TEST_F(SyllableParserTest, TaiText_Ho2) {
    auto raw = u8"ho";
    auto target = u8"Hó";
    auto result = parser->AsTaiText(raw, target);
    EXPECT_EQ(result.raw(), "ho");
    EXPECT_EQ(result.composed(), "ho");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(SyllableParserTest, Erase_a) {
    auto s = Syllable();
    s.composed = "a";
    s.raw_input = "a";
    s.raw_body = "a";
    parser->Erase(s, 0);
    EXPECT_EQ(s.composed.size(), 0);
    EXPECT_EQ(s.raw_body.size(), 0);
    EXPECT_EQ(s.raw_input.size(), 0);
}

TEST_F(SyllableParserTest, Erase_kah8) {
    auto s = Syllable();
    s.composed = u8"ka\u030dh";
    s.raw_input = "kah8";
    s.raw_body = "kah";
    parser->Erase(s, 1);
    EXPECT_EQ(s.composed, "kh");
    EXPECT_EQ(s.raw_body, "kh");
    EXPECT_EQ(s.raw_input, "kh");
}

TEST_F(SyllableParserTest, Khin_a) {
    auto input = "--a";
    auto s = Syllable();
    parser->ParseRaw(input, s);
    EXPECT_EQ(s.composed, "·a");
    EXPECT_EQ(s.raw_body, "a");
    EXPECT_EQ(s.raw_input, "--a");
    EXPECT_EQ(s.khin_pos, KhinKeyPosition::Start);
    EXPECT_EQ(s.khin_key, '-');
}

TEST_F(SyllableParserTest, Khin_a0) {
    auto input = "a0";
    auto s = Syllable();
    parser->ParseRaw(input, s);
    EXPECT_EQ(s.composed, "·a");
    EXPECT_EQ(s.raw_body, "a");
    EXPECT_EQ(s.raw_input, "a0");
    EXPECT_EQ(s.khin_pos, KhinKeyPosition::End);
    EXPECT_EQ(s.khin_key, '0');
}

TEST_F(SyllableParserTest, AsTaiText_sou2i2) {
    auto input = u8"sou2i2";
    auto target = u8"só͘ í";
    auto result = parser->AsTaiText(input, target);
    auto tt = TaiText();
    tt.AddItem(Syllable{"sou2", "sou", Tone::T2, KhinKeyPosition::None, '2', 0, "só͘"});
    tt.AddItem(VirtualSpace());
    tt.AddItem(Syllable{"i2", "i", Tone::T2, KhinKeyPosition::None, '2', 0, "í"});
    EXPECT_EQ(result, tt);
}

} // namespace
} // namespace khiin::engine

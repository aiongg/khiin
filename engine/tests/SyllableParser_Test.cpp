#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "KeyConfig.h"
#include "Syllable.h"
#include "SyllableParser.h"
#include "BufferSegment.h"

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

TEST(SyllableParserTest, ParseRawTest) {
    auto input = "ho2";
    auto syl = Syllable();
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "ho");
    EXPECT_EQ(syl.tone, Tone::T2);
    EXPECT_EQ(syl.tone_key, '2');
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::None);
    EXPECT_EQ(syl.composed, u8"hó");
    EXPECT_EQ(syl.raw_body.size(), 2);
}

TEST(SyllableParserTest, ParseRawTest2) {
    auto input = "hou7";
    auto syl = Syllable();
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);

    parser->ParseRaw(input, syl);

    EXPECT_EQ(syl.raw_body, "hou");
    EXPECT_EQ(syl.tone, Tone::T7);
    EXPECT_EQ(syl.tone_key, '7');
    EXPECT_EQ(syl.khin_pos, KhinKeyPosition::None);
    EXPECT_EQ(syl.composed, u8"hō\u0358");
}

TEST(SyllableParserTest, ConvertCaretPosition) {
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto syl = Syllable();
    syl.raw_input = "ho2";
    syl.raw_body = "ho";
    syl.tone = Tone::T2;
    syl.tone_key = '2';
    syl.composed = u8"hó";
    size_t caret = std::string::npos;

    parser->RawToComposedCaret(syl, 0, caret);
    EXPECT_EQ(caret, 0);
    parser->RawToComposedCaret(syl, 1, caret);
    EXPECT_EQ(caret, 1);
    parser->RawToComposedCaret(syl, 3, caret);
    EXPECT_EQ(caret, 2);
    parser->RawToComposedCaret(syl, 4, caret);
    EXPECT_EQ(caret, std::string::npos);


    parser->ComposedToRawCaret(syl, 0, caret);
    EXPECT_EQ(caret, 0);
    parser->ComposedToRawCaret(syl, 1, caret);
    EXPECT_EQ(caret, 1);
    parser->ComposedToRawCaret(syl, 2, caret);
    EXPECT_EQ(caret, 3);
    parser->ComposedToRawCaret(syl, 3, caret);
    EXPECT_EQ(caret, std::string::npos);
}

TEST(SyllableParserTest, ConvertCaretPosition2) {
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto syl = Syllable();
    syl.raw_input = "hounn3";
    syl.raw_body = "hounn";
    syl.tone = Tone::T3;
    syl.tone_key = '3';
    syl.composed = u8"hò\u0358ⁿ";
    size_t caret = std::string::npos;

    parser->RawToComposedCaret(syl, 0, caret);
    EXPECT_EQ(caret, 0);
    parser->RawToComposedCaret(syl, 1, caret);
    EXPECT_EQ(caret, 1);
    parser->RawToComposedCaret(syl, 3, caret);
    EXPECT_EQ(caret, 3);
    parser->RawToComposedCaret(syl, 6, caret);
    EXPECT_EQ(caret, 4);
    parser->RawToComposedCaret(syl, 7, caret);
    EXPECT_EQ(caret, std::string::npos);

    parser->ComposedToRawCaret(syl, 0, caret);
    EXPECT_EQ(caret, 0);
    parser->ComposedToRawCaret(syl, 1, caret);
    EXPECT_EQ(caret, 1);
    parser->ComposedToRawCaret(syl, 3, caret);
    EXPECT_EQ(caret, 3);
    parser->ComposedToRawCaret(syl, 4, caret);
    EXPECT_EQ(caret, 6);
    parser->ComposedToRawCaret(syl, 5, caret);
    EXPECT_EQ(caret, std::string::npos);
}

TEST(SyllableParserTest, ToFuzzy) {
    auto input = u8"hō\u0358";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = std::vector<std::string>();
    auto has_tone = false;
    parser->ToFuzzy(input, result, has_tone);

    EXPECT_THAT(result, Contains("hou7"));
    EXPECT_TRUE(has_tone);
}

TEST(SyllableParserTest, AsInputSequences) {
    auto input = u8"pêng-an";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = input_sequence_strings_only(parser->AsInputSequences(input));

    EXPECT_THAT(result, Contains(u8"pengan"));
    EXPECT_THAT(result, Contains(u8"peng-an"));
    EXPECT_THAT(result, Contains(u8"peng5an"));
    EXPECT_THAT(result, Contains(u8"peng5-an"));

    input = u8"a-bí-cho\u030dk";
    result = input_sequence_strings_only(parser->AsInputSequences(input));
    EXPECT_EQ(result.size(), 16);
    EXPECT_THAT(result, Contains("abichok"));
}

TEST(SyllableParserTest, AsInputSequencesWithKhin) {
    auto input = u8"lo\u030dh-·khì";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = input_sequence_strings_only(parser->AsInputSequences(input));

    EXPECT_THAT(result, Contains(u8"lohkhi"));
    EXPECT_THAT(result, Contains(u8"lohkhi3"));
    EXPECT_THAT(result, Contains(u8"loh8khi"));
    EXPECT_THAT(result, Contains(u8"loh8khi3"));
    EXPECT_THAT(result, Contains(u8"loh-khi"));
    EXPECT_THAT(result, Contains(u8"loh-khi3"));
    EXPECT_THAT(result, Contains(u8"loh8-khi"));
    EXPECT_THAT(result, Contains(u8"loh8-khi3"));
}

TEST(SyllableParserTest, AsInputsSingleSylWithTone) {
    auto input = u8"pêng";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = parser->AsInputSequences(input);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].input, "peng5");
    EXPECT_FALSE(result[0].is_fuzzy_monosyllable);
    EXPECT_EQ(result[1].input, "peng");
    EXPECT_TRUE(result[1].is_fuzzy_monosyllable);
}

TEST(SyllableParserTest, AsInputsNoSpace) {
    auto input = u8"pêng an";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = parser->AsInputSequences(input);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].input, "peng5an");
    EXPECT_EQ(result[1].input, "pengan");
}

TEST(SyllableParserTest, AsBufferSegment) {
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto raw = "pengan";
    auto target = u8"pêng-an";
    
    auto result = parser->AsBufferSegment(raw, target);
    EXPECT_EQ(result.Raw(), "pengan");
    EXPECT_EQ(result.Display(), "peng an");
    EXPECT_EQ(result.Size(), 7);
}

} // namespace
} // namespace khiin::engine

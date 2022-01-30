#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "KeyConfig.h"
#include "Syllable.h"
#include "SyllableParser.h"

namespace khiin::engine {
namespace {

using ::testing::Contains;

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

TEST(SyllableParserTest, ToRaw) {
    auto input = u8"hō\u0358";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = std::vector<std::string>();
    auto has_tone = false;
    parser->ToRaw(input, result, has_tone);

    EXPECT_THAT(result, Contains("hou7"));
    EXPECT_TRUE(has_tone);
}

TEST(SyllableParserTest, MultisylInputSequences) {
    auto input = u8"pêng-an";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = parser->GetMultisylInputSequences(input);

    EXPECT_THAT(result, Contains(u8"pengan"));
    EXPECT_THAT(result, Contains(u8"peng-an"));
    EXPECT_THAT(result, Contains(u8"peng5an"));
    EXPECT_THAT(result, Contains(u8"peng5-an"));
}

TEST(SyllableParserTest, MultisylInputSequencesWithKhin) {
    auto input = u8"lo\u030dh-·khì";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = parser->GetMultisylInputSequences(input);

    EXPECT_THAT(result, Contains(u8"lohkhi"));
    EXPECT_THAT(result, Contains(u8"lohkhi3"));
    EXPECT_THAT(result, Contains(u8"loh8khi"));
    EXPECT_THAT(result, Contains(u8"loh8khi3"));
    EXPECT_THAT(result, Contains(u8"loh-khi"));
    EXPECT_THAT(result, Contains(u8"loh-khi3"));
    EXPECT_THAT(result, Contains(u8"loh8-khi"));
    EXPECT_THAT(result, Contains(u8"loh8-khi3"));
}

TEST(SyllableParserTest, MultisylInputSingleSylWithTone) {
    auto input = u8"pêng";
    auto keyconfig = KeyConfig::Create();
    auto parser = SyllableParser::Create(keyconfig);
    auto result = parser->GetMultisylInputSequences(input);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "peng5");
}

} // namespace
} // namespace khiin::engine

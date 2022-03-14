#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "proto/proto.h"

#include "Engine.h"
#include "KeyConfig.h"
#include "Syllable.h"
#include "SyllableParser.h"
#include "TaiText.h"
#include "TestEnv.h"

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
        parser = TestEnv::engine()->syllable_parser();
    }
    SyllableParser *parser = nullptr;
};

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
    EXPECT_EQ(result.RawText(), "pengan");
    EXPECT_EQ(result.ComposedText(), "peng an");
    EXPECT_EQ(result.ComposedSize(), 7);
}

TEST_F(SyllableParserTest, TaiText_Ho2) {
    auto raw = u8"ho";
    auto target = u8"Hó";
    auto result = parser->AsTaiText(raw, target);
    EXPECT_EQ(result.RawText(), "ho");
    EXPECT_EQ(result.ComposedText(), "ho");
    EXPECT_EQ(result.ComposedSize(), 2);
}

TEST_F(SyllableParserTest, AsTaiText_sou2i2) {
    auto input = u8"sou2i2";
    auto target = u8"só\u0358 í";
    auto result = parser->AsTaiText(input, target);
    auto tt = TaiText();
    tt.AddItem(parser->ParseRaw("sou2"));
    tt.AddItem(VirtualSpace());
    tt.AddItem(parser->ParseRaw("i2"));
    EXPECT_EQ(result, tt);
}

TEST_F(SyllableParserTest, TaiText_capitals) {
    auto input = "Goa2";
    auto target = "góa";
    auto result = parser->AsTaiText(input, target);
    EXPECT_EQ(result.RawText(), "Goa2");
    EXPECT_EQ(result.ComposedText(), "Góa");
}

TEST_F(SyllableParserTest, TaiText_an1) {
    auto input = "an1";
    auto target = "an";
    auto result = parser->AsTaiText(input, target);
    EXPECT_EQ(result.RawText(), "an1");
    EXPECT_EQ(result.ComposedText(), "an");
}

} // namespace
} // namespace khiin::engine

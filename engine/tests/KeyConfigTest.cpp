#include <gtest/gtest.h>

#include "KeyConfig.h"

namespace khiin::engine {
namespace {

TEST(KeyConfigTest, SetNasalKey) {
    auto kc = KeyConfig::CreateEmpty();
    auto ret = kc->SetKey('n', VKey::Nasal, false);
    EXPECT_TRUE(ret);
    auto rules = kc->ConversionRules();
    EXPECT_EQ(rules.size(), 4);
    EXPECT_EQ(rules[0].first, "nn");
    EXPECT_EQ(rules[0].second, u8"\u207f");
    EXPECT_EQ(rules[1].first, "nN");
    EXPECT_EQ(rules[1].second, u8"\u207f");
    EXPECT_EQ(rules[2].first, "Nn");
    EXPECT_EQ(rules[2].second, u8"\u1d3a");
    EXPECT_EQ(rules[3].first, "NN");
    EXPECT_EQ(rules[3].second, u8"\u1d3a");
}

TEST(KeyConfigTest, SetStandaloneNasalKey) {
    auto kc = KeyConfig::CreateEmpty();
    auto ret = kc->SetKey('v', VKey::Nasal, true);
    EXPECT_TRUE(ret);
    auto rules = kc->ConversionRules();
    EXPECT_EQ(rules.size(), 2);
    EXPECT_EQ(rules[0].first, "v");
    EXPECT_EQ(rules[0].second, u8"\u207f");
    EXPECT_EQ(rules[1].first, "V");
    EXPECT_EQ(rules[1].second, u8"\u1d3a");
}

TEST(KeyConfigTest, SetInvalidNasalKey) {
    auto kc = KeyConfig::CreateEmpty();
    auto ret = kc->SetKey('a', VKey::Nasal);
    EXPECT_FALSE(ret);
}

TEST(KeyConfigTest, SetDotAboveKey) {
    auto kc = KeyConfig::CreateEmpty();
    auto ret = kc->SetKey('u', VKey::DotAboveRight, false);
    EXPECT_TRUE(ret);
    auto rules = kc->ConversionRules();
    EXPECT_EQ(rules.size(), 4);
    EXPECT_EQ(rules[0].first, "ou");
    EXPECT_EQ(rules[0].second, u8"o\u0358");
    EXPECT_EQ(rules[1].first, "oU");
    EXPECT_EQ(rules[1].second, u8"o\u0358");
    EXPECT_EQ(rules[2].first, "Ou");
    EXPECT_EQ(rules[2].second, u8"O\u0358");
    EXPECT_EQ(rules[3].first, "OU");
    EXPECT_EQ(rules[3].second, u8"O\u0358");
}

TEST(KeyConfigTest, SetStandaloneDotAboveKey) {
    auto kc = KeyConfig::CreateEmpty();
    auto ret = kc->SetKey('w', VKey::DotAboveRight, true);
    EXPECT_TRUE(ret);
    auto rules = kc->ConversionRules();
    EXPECT_EQ(rules.size(), 2);
    EXPECT_EQ(rules[0].first, "w");
    EXPECT_EQ(rules[0].second, u8"o\u0358");
    EXPECT_EQ(rules[1].first, "W");
    EXPECT_EQ(rules[1].second, u8"O\u0358");
}

TEST(KeyConfigTest, HasFallbackToneDigits) {
    auto kc = KeyConfig::CreateEmpty();
    auto tone = kc->CheckToneKey('2');
    EXPECT_EQ(tone, Tone::T2);
}

} // namespace
} // namespace khiin::engine
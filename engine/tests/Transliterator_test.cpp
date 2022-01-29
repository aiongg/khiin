#include <gtest/gtest.h>

#include "Transliterator.h"

#include <algorithm>

#include "Syllable.h"

namespace khiin::engine {
namespace {

class TransliteratorTest : public ::testing::Test {
  protected:
    void SetUp() override {
        t = Transliterator::default_instance();
    }
    Transliterator *t = nullptr;
};

TEST_F(TransliteratorTest, BasicConversions) {
    std::string in, out, res;
    ToneInfo ti = {};

    in = "hou";
    out = "ho\u0358";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "ann";
    out = "a\u207f";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "hur";
    out = u8"h\u1e73";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "HOU";
    out = "HO\u0358";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "ANN";
    out = "A\u1d3a";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "HUR";
    out = "H\u1e72";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "HOu";
    out = "HO\u0358";
    res = t->Precompose(in, ti);
    in = "HOU";
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "ANn";
    out = "A\u1d3a";
    res = t->Precompose(in, ti);
    in = "ANN";
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);

    in = "HUr";
    out = "H\u1e72";
    res = t->Precompose(in, ti);
    in = "HUR";
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(in, res);
}

TEST_F(TransliteratorTest, ToneConversion) {
    std::string in, out, res;

    ToneInfo ti = {Tone::T2, '2', KhinKeyPosition::None, 0};
    in = "ho";
    out = "hó";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(res, "ho2");

    ti.tone = Tone::T3;
    ti.tone_key = '3';
    in = "ho";
    out = "hò";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(res, "ho3");

    ti.tone = Tone::T5;
    ti.tone_key = '5';
    in = "ho";
    out = "hô";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(res, "ho5");

    ti.tone = Tone::T7;
    ti.tone_key = '7';
    in = "ho";
    out = "hō";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(res, "ho7");

    ti.tone = Tone::T8;
    ti.tone_key = '8';
    in = "ho";
    out = "ho̍";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(res, "ho8");

    ti.tone = Tone::T9;
    ti.tone_key = '9';
    in = "ho";
    out = "hŏ";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ(res, "ho9");
}

TEST_F(TransliteratorTest, ToneWithSpecials) {
    std::string in, out, res;
    ToneInfo ti = {Tone::T3, '3', KhinKeyPosition::None, 0};

    in = "ann";
    out = "à\u207f";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("ann3", res);

    in = "hounn";
    out = "hò\u0358\u207f";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("hounn3", res);

    in = "hur";
    out = "h\u1e73\u0300";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("hur3", res);
}

TEST_F(TransliteratorTest, OtherKeys) {
    auto t = Transliterator::Create();
    t->AddConversionRule("w", "o\u0358");
    t->AddConversionRule("W", "O\u0358");
    t->AddConversionRule("v", "\u207f");
    t->AddConversionRule("V", "\u1d3a");
    t->AddToneKey(Tone::T2, '2');

    ToneInfo ti = {Tone::T2, '2', KhinKeyPosition::None, 0};
    std::string in, out, res;
    in = "hw";
    out = "hó\u0358";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("hw2", res);

    in = "HW";
    out = "HÓ\u0358";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("HW2", res);

    in = "av";
    out = "á\u207f";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("av2", res);

    in = "AV";
    out = "Á\u1d3a";
    res = t->Precompose(in, ti);
    EXPECT_EQ(out, res);
    res = t->Decompose(out, ti);
    EXPECT_EQ("AV2", res);
}

//TEST_F(TransliteratorTest, InsertKey) {
//    auto s_in = std::string();
//    auto s_out = std::string();
//    auto i = 0;
//    auto j = -1;
//
//    t->InsertAt(s_in, i, 'k', s_out, j);
//    EXPECT_EQ(s_out, "k");
//    EXPECT_EQ(j, 1);
//
//    s_in = s_out;
//    i = j;
//    t->InsertAt(s_in, i, 'i', s_out, j);
//    EXPECT_EQ(s_out, "ki");
//    EXPECT_EQ(j, 2);
//
//    s_in = s_out;
//    i = j;
//    t->InsertAt(s_in, i, '2', s_out, j);
//    EXPECT_EQ(s_out, u8"kí");
//    EXPECT_EQ(j, 2);
//
//    s_in = s_out;
//    i = j;
//    t->InsertAt(s_in, i, 'u', s_out, j);
//    EXPECT_EQ(s_out, u8"kiú");
//    EXPECT_EQ(j, 3);
//}

TEST_F(TransliteratorTest, RawBufferSize) {
    auto composed = std::string("a");
    auto index = 1;

    auto lhs_size = t->DecomposedSize(composed, index);
    EXPECT_EQ(lhs_size, 1);

    composed = u8"ò͘ⁿh"; // LHS: ounn3   RHS: h
    index = 3;
    lhs_size = t->DecomposedSize(composed, index);
    EXPECT_EQ(lhs_size, 4);

    composed = u8"·a"; // --    0
}

} // namespace
} // namespace khiin::engine

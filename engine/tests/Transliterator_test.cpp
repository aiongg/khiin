#include <gtest/gtest.h>

#include <algorithm>

#include "Transliterator.h"

namespace khiin::engine {
namespace {

class TransliteratorTest : public ::testing::Test {
  protected:
    void SetUp() override {
        t = Transliterator::Create();
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
    }
    Transliterator *t = nullptr;
};

TEST_F(TransliteratorTest, BasicConversions) {
    std::string in, out, res;

    in = "hou";
    out = "ho\u0358";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ann";
    out = "a\u207f";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "hur";
    out = u8"h\u1e73";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "HOU";
    out = "HO\u0358";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ANN";
    out = "A\u1d3a";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "HUR";
    out = "H\u1e72";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "HOu";
    out = "HO\u0358";
    res = t->Precompose(in);
    in = "HOU";
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ANn";
    out = "A\u1d3a";
    res = t->Precompose(in);
    in = "ANN";
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "HUr";
    out = "H\u1e72";
    res = t->Precompose(in);
    in = "HUR";
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);
}

TEST_F(TransliteratorTest, ToneConversion) {
    std::string in, out, res;

    in = "ho2";
    out = "hó";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ho3";
    out = "hò";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ho5";
    out = "hô";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ho7";
    out = "hō";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ho8";
    out = "ho̍";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "ho9";
    out = "hŏ";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);
}

TEST_F(TransliteratorTest, ToneWithSpecials) {
    std::string in, out, res;

    in = "ann3";
    out = "à\u207f";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "hounn3";
    out = "hò\u0358\u207f";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "hur5";
    out = "h\u1e73\u0302";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);
}

TEST_F(TransliteratorTest, OtherKeys) {
    auto t = Transliterator::Create();
    t->AddConversionRule("w", "o\u0358");
    t->AddConversionRule("W", "O\u0358");
    t->AddConversionRule("v", "\u207f");
    t->AddConversionRule("V", "\u1d3a");
    t->AddToneKey(Tone::T2, '2');

    std::string in, out, res;
    in = "hw2";
    out = "hó\u0358";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "HW2";
    out = "HÓ\u0358";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "av2";
    out = "á\u207f";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);

    in = "AV2";
    out = "Á\u1d3a";
    res = t->Precompose(in);
    EXPECT_EQ(out, res);
    res = t->Decompose(out);
    EXPECT_EQ(in, res);
}

TEST_F(TransliteratorTest, InsertKey) {
    auto s = std::string();
    auto i = 0;

    t->InsertAt(s, i, 'k');

    EXPECT_EQ(s, "k");
    EXPECT_EQ(i, 1);

    t->InsertAt(s, i, 'i');

    EXPECT_EQ(s, "ki");
    EXPECT_EQ(i, 2);

    t->InsertAt(s, i, '2');

    EXPECT_EQ(s, u8"kí");
    EXPECT_EQ(i, 2);

    t->InsertAt(s, i, 'u');

    EXPECT_EQ(s, u8"kiú");
    EXPECT_EQ(i, 3);
}

} // namespace
} // namespace khiin::engine

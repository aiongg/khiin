#include <gtest/gtest.h>

#include "Engine.h"
#include "config/KeyConfig.h"
#include "input/Syllable.h"

#include "TestEnv.h"

namespace khiin::engine::test {

struct SyllableTest : ::testing::Test {
  protected:
    void SetUp() {
        config = KeyConfig::Create();
    }

    Syllable FromRaw(std::string raw, bool dotted_khin = true) {
        auto s = Syllable(config.get(), dotted_khin);
        s.SetRawInput(raw);
        return s;
    }

    void ExpectSyllable(Syllable &syl, std::string composed, std::string raw_input, std::string raw_body,
                        Tone tone = Tone::NaT, char tone_key = 0, KhinKeyPosition khin_pos = KhinKeyPosition::None,
                        char khin_key = 0) {
        EXPECT_EQ(syl.composed(), composed);
        EXPECT_EQ(syl.raw_input(), raw_input);
        EXPECT_EQ(syl.raw_body(), raw_body);
        EXPECT_EQ(syl.tone(), tone);
        EXPECT_EQ(syl.tone_key(), tone_key);
        EXPECT_EQ(syl.khin_pos(), khin_pos);
        EXPECT_EQ(syl.khin_key(), khin_key);
    }

    void ExpectRawCaret(Syllable &syl, size_t raw_caret, size_t composed_caret) {
        EXPECT_EQ(syl.RawToComposedCaret(raw_caret), composed_caret);
    }

    void ExpectComposedCaret(Syllable &syl, size_t composed_caret, size_t raw_caret) {
        EXPECT_EQ(syl.ComposedToRawCaret(composed_caret), raw_caret);
    }

    std::unique_ptr<KeyConfig> config;
};

TEST_F(SyllableTest, Parse_ho2) {
    auto syl = FromRaw("ho2");
    ExpectSyllable(syl, "hó", "ho2", "ho", Tone::T2, '2');
}

TEST_F(SyllableTest, Parse_hou7) {
    auto syl = FromRaw("hou7");
    ExpectSyllable(syl, "hō͘", "hou7", "hou", Tone::T7, '7');
}

TEST_F(SyllableTest, Parse_an1) {
    auto syl = FromRaw("an1");
    ExpectSyllable(syl, "an", "an1", "an", Tone::T1, '1');
}

TEST_F(SyllableTest, Parse_bah4) {
    auto syl = FromRaw("bah4");
    ExpectSyllable(syl, "bah", "bah4", "bah", Tone::T4, '4');
}

TEST_F(SyllableTest, Parse_ouhnn8) {
    auto syl = FromRaw("ouhnn8");
    ExpectSyllable(syl, "o\u030d\u0358h\u207f", "ouhnn8", "ouhnn", Tone::T8, '8');
}

TEST_F(SyllableTest, Parse_ian9) {
    auto syl = FromRaw("ian9");
    ExpectSyllable(syl, "iăn", "ian9", "ian", Tone::T9, '9');
}

TEST_F(SyllableTest, Parse_mng5) {
    auto syl = FromRaw("mng5");
    ExpectSyllable(syl, "mn\u0302g", "mng5", "mng", Tone::T5, '5');
}

TEST_F(SyllableTest, Parse_hur5) {
    auto syl = FromRaw("hur5");
    ExpectSyllable(syl, "hṳ\u0302", "hur5", "hur", Tone::T5, '5');
}

TEST_F(SyllableTest, Parse_hor3) {
    auto syl = FromRaw("hor3");
    ExpectSyllable(syl, "hò\u0324", "hor3", "hor", Tone::T3, '3');
}

TEST_F(SyllableTest, Parse_khin) {
    auto syl = FromRaw("--");
    ExpectSyllable(syl, "\u00b7", "--", "", Tone::NaT, 0, KhinKeyPosition::Start, '-');
}

TEST_F(SyllableTest, Convert_caret_ho2) {
    auto syl = FromRaw("ho2");
    ExpectRawCaret(syl, 0, 0);
    ExpectRawCaret(syl, 1, 1);
    ExpectRawCaret(syl, 3, 2);
    ExpectRawCaret(syl, 4, std::string::npos);
    ExpectComposedCaret(syl, 0, 0);
    ExpectComposedCaret(syl, 1, 1);
    ExpectComposedCaret(syl, 2, 3);
    ExpectComposedCaret(syl, 3, std::string::npos);
}

TEST_F(SyllableTest, Convert_caret_hounn3) {
    auto syl = FromRaw("hounn3");
    ExpectRawCaret(syl, 0, 0);
    ExpectRawCaret(syl, 1, 1);
    ExpectRawCaret(syl, 3, 3);
    ExpectRawCaret(syl, 6, 4);
    ExpectRawCaret(syl, 7, std::string::npos);
    ExpectComposedCaret(syl, 0, 0);
    ExpectComposedCaret(syl, 1, 1);
    ExpectComposedCaret(syl, 3, 3);
    ExpectComposedCaret(syl, 4, 6);
    ExpectComposedCaret(syl, 5, std::string::npos);
}

TEST_F(SyllableTest, Erase_a) {
    auto syl = FromRaw("a");
    syl.Erase(0);
    ExpectSyllable(syl, "", "", "", Tone::NaT, 0, KhinKeyPosition::None, 0);
}

TEST_F(SyllableTest, Erase_kah8) {
    auto syl = FromRaw("kah8");
    syl.Erase(1);
    ExpectSyllable(syl, "kh", "kh", "kh", Tone::NaT, 0);
}

TEST_F(SyllableTest, Erase_khin) {
    auto syl = FromRaw("--a");
    syl.Erase(1);
    ExpectSyllable(syl, "·", "--", "", Tone::NaT, 0, KhinKeyPosition::Start, '-');
    syl.Erase(0);
    EXPECT_TRUE(syl.Empty());
}

TEST_F(SyllableTest, Parse_khin_only) {
    auto syl = FromRaw("--");
    ExpectSyllable(syl, "·", "--", "", Tone::NaT, 0, KhinKeyPosition::Start, '-');
}
TEST_F(SyllableTest, Khin_a) {
    auto syl = FromRaw("--a");
    ExpectSyllable(syl, "·a", "--a", "a", Tone::NaT, 0, KhinKeyPosition::Start, '-');
    ExpectRawCaret(syl, 0, 0);
    ExpectRawCaret(syl, 2, 1);
    ExpectRawCaret(syl, 3, 2);
}

TEST_F(SyllableTest, Khin_a0) {
    auto syl = FromRaw("a0");
    ExpectSyllable(syl, "·a", "a0", "a", Tone::NaT, 0, KhinKeyPosition::End, '0');
}

TEST_F(SyllableTest, Khin_with_dashes) {
    auto input = "--a";
    auto syl = FromRaw("--a", false);
    ExpectSyllable(syl, "--a", "--a", "a", Tone::NaT, 0, KhinKeyPosition::Start, '-');
}

} // namespace khiin::engine::test
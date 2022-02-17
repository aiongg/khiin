#include <gtest/gtest.h>

//#include "common.h"
#include "Lomaji.h"

namespace khiin::engine {
namespace {

TEST(Lomaji, QuickDecompose) {
    auto in = u8"á";
    auto out = Lomaji::Decompose(in);
    EXPECT_EQ(out, u8"a\u0301");

    in = u8"áàâāa̍ăó͘ṳ";
    out = Lomaji::Decompose(in);
    EXPECT_STREQ(out.data(), u8"a\u0301a\u0300a\u0302a\u0304a\u030da\u0306o\u0301\u0358u\u0324");
}

// TEST(Lomaji, place_tone_on_syllable) {
//    std::string ret = placeToneOnSyllable(u8"oan", Tone::T2);
//    EXPECT_EQ(ret, u8"oa\u0301n");
//}
//
// TEST(Lomaji, utf8_to_ascii) {
//    std::string ret = utf8ToAsciiLower("á");
//    EXPECT_EQ(ret, "a2");
//
//    ret = utf8ToAsciiLower("A-bí-cho̍k");
//    EXPECT_EQ(ret, "a-bi2-chok8");
//
//    ret = utf8ToAsciiLower("àⁿ lo̍h khì");
//    EXPECT_EQ(ret, "ann3 loh8 khi3");
//
//    ret = utf8ToAsciiLower("siak ·lo̍h-·khì bān-té chhim-kheⁿ");
//    EXPECT_EQ(ret, "siak loh80-khi30 ban7-te2 chhim-khenn");
//}
//
// TEST(Lomaji, split_ascii_by_utf8) {
//    auto r = spaceAsciiByUtf8("khiam3eng7", u8"khiàm-ēng");
//    EXPECT_EQ(r.size(), 2);
//    EXPECT_EQ(r[0], "khiam3");
//    EXPECT_EQ(r[1], "eng7");
//
//    r = spaceAsciiByUtf8("khouounnla0", u8"khó͘-ò͘ⁿ ·la");
//    EXPECT_EQ(r.size(), 3);
//    EXPECT_EQ(r[0], "khou");
//    EXPECT_EQ(r[1], "ounn");
//    EXPECT_EQ(r[2], "la0");
//
//    r = spaceAsciiByUtf8("oun", u8"ò-ūn");
//    EXPECT_EQ(r.size(), 2);
//    EXPECT_EQ(r[0], "o");
//    EXPECT_EQ(r[1], "un");
//
//    r = spaceAsciiByUtf8("unna", u8"ûn-ná");
//    EXPECT_EQ(r.size(), 2);
//    EXPECT_EQ(r[0], "un");
//    EXPECT_EQ(r[1], "na");
//
//    r = spaceAsciiByUtf8("khiam3-eng7", u8"khiàm-ēng");
//    EXPECT_EQ(r.size(), 2);
//    EXPECT_EQ(r[0], "khiam3-");
//    EXPECT_EQ(r[1], "eng7");
//}

} // namespace
} // namespace khiin::engine

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "common.h"
#include "lomaji.h"

using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(LomajiTest);

BOOST_AUTO_TEST_CASE(place_tone_on_syllable) {
    std::string ret = placeToneOnSyllable(u8"oan", Tone::T2);
    BOOST_TEST(ret == u8"oa\u0301n");
}

BOOST_AUTO_TEST_CASE(utf8_to_ascii) {
    std::string ret = utf8ToAsciiLower("á");
    BOOST_TEST(ret == "a2");

    ret = utf8ToAsciiLower("A-bí-cho̍k");
    BOOST_TEST(ret == "a-bi2-chok8");

    ret = utf8ToAsciiLower("àⁿ lo̍h khì");
    BOOST_TEST(ret == "ann3 loh8 khi3");

    ret = utf8ToAsciiLower("siak ·lo̍h-·khì bān-té chhim-kheⁿ");
    BOOST_TEST(ret == "siak loh80-khi30 ban7-te2 chhim-khenn");
}

BOOST_AUTO_TEST_CASE(ascii_cursor_from_utf8) {
    std::string ascii = u8"oan5";
    std::string u8str = asciiSyllableToUtf8(ascii, Tone::T5, false);
    size_t ret;

    ret = getAsciiCursorFromUtf8(ascii, u8str, 0);
    BOOST_TEST(ret == 0);

    ret = getAsciiCursorFromUtf8(ascii, u8str, 1);
    BOOST_TEST(ret == 1);

    ret = getAsciiCursorFromUtf8(ascii, u8str, 2);
    BOOST_TEST(ret == 2);

    ret = getAsciiCursorFromUtf8(ascii, u8str, 3);
    BOOST_TEST(ret == 3);

    ascii = u8"hounn3";
    u8str = u8"hò͘ⁿ";

    ret = getAsciiCursorFromUtf8(ascii, u8str, 0);
    BOOST_TEST(ret == 0);

    ret = getAsciiCursorFromUtf8(ascii, u8str, 1);
    BOOST_TEST(ret == 1);

    ret = getAsciiCursorFromUtf8(ascii, u8str, 3);
    BOOST_TEST(ret == 3);

    ret = getAsciiCursorFromUtf8(ascii, u8str, 4);
    BOOST_TEST(ret == 5);
}

BOOST_AUTO_TEST_CASE(split_ascii_by_utf8) {
    auto r = spaceAsciiByUtf8("khiam3eng7", u8"khiàm-ēng");
    BOOST_TEST(r.size() == 2);
    BOOST_TEST(r[0] == "khiam3");
    BOOST_TEST(r[1] == "eng7");

    r = spaceAsciiByUtf8("khouounnla0", u8"khó͘-ò͘ⁿ ·la");
    BOOST_TEST(r.size() == 3);
    BOOST_TEST(r[0] == "khou");
    BOOST_TEST(r[1] == "ounn");
    BOOST_TEST(r[2] == "la0");

    r = spaceAsciiByUtf8("oun", u8"ò-ūn");
    BOOST_TEST(r.size() == 2);
    BOOST_TEST(r[0] == "o");
    BOOST_TEST(r[1] == "un");

    r = spaceAsciiByUtf8("unna", u8"ûn-ná");
    BOOST_TEST(r.size() == 2);
    BOOST_TEST(r[0] == "un");
    BOOST_TEST(r[1] == "na");

    r = spaceAsciiByUtf8("khiam3-eng7", u8"khiàm-ēng");
    BOOST_TEST(r.size() == 3);
    BOOST_TEST(r[0] == "khiam3");
    BOOST_TEST(r[1] == "-");
    BOOST_TEST(r[2] == "eng7");
}

BOOST_AUTO_TEST_SUITE_END();

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

BOOST_AUTO_TEST_CASE(ascii_cursor_from_utf8) {
    std::string ascii = u8"oan5";
    std::string u8str = asciiToUtf8(ascii, Tone::T5, false);
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

BOOST_AUTO_TEST_SUITE_END();

// libtaikey_test.cpp : Source file for your target.
//

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <vector>

#include "libtaikey.h"

using namespace std;
using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(LibtaikeyTest);

typedef pair<string, string> test_w;

struct Fx {
    Fx() {
        words.push_back(test_w(u8"a", u8"a"));
        words.push_back(test_w(u8"as", u8"á"));
        words.push_back(test_w(u8"af", u8"à"));
        words.push_back(test_w(u8"uif", u8"ùi"));
        words.push_back(test_w(u8"uis", u8"úi"));
        words.push_back(test_w(u8"iuj", u8"iū"));
        words.push_back(test_w(u8"boal", u8"bôa"));
        words.push_back(test_w(u8"oanl", u8"oân"));
        words.push_back(test_w(u8"aw", u8"ă"));
        words.push_back(test_w(u8"khoannf", u8"khòaⁿ"));
    }
    ~Fx() { words.clear(); }

    vector<test_w> words;
};

BOOST_GLOBAL_FIXTURE(Fx);

void feedKeys(TKEngine &e, const char *keys) {
    e.reset();
    for (auto c : std::string(keys)) {
        e.onKeyDown(c);
    }
}
BOOST_AUTO_TEST_CASE(Engine_Empty) {
    TKEngine e;
    e.reset();
    BOOST_CHECK_EQUAL(u8"", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Buffer) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"a");
    BOOST_CHECK_EQUAL(u8"a", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Tone) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"as");
    BOOST_CHECK_EQUAL(u8"á", e.getBuffer());
    e.reset();
    feedKeys(e, u8"af");
    BOOST_CHECK_EQUAL(u8"à", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_TonePlacement) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"uis");
    BOOST_CHECK_EQUAL(u8"úi", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Failure, *boost::unit_test::expected_failures(1)) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"a");
    BOOST_CHECK_EQUAL(u8"b", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Words) {
    vector<string> expected;
    vector<string> actual;

    TKEngine e;
    Fx f;

    for (test_w word : f.words) {
        e.reset();
        feedKeys(e, word.first.c_str());
        expected.push_back(word.second);
        actual.push_back(e.getBuffer());
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(Engine_OU) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"ou");
    BOOST_CHECK_EQUAL(u8"o͘", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Nasal) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"ann");
    BOOST_CHECK_EQUAL(u8"a\u207f", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_OU_Tone) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"ous");
    BOOST_CHECK_EQUAL(u8"ó͘", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Nasal_Tone) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"annf");
    BOOST_CHECK_EQUAL(u8"à\u207f", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_OU_ToneChange) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"ousf");
    BOOST_CHECK_EQUAL(u8"ò͘", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Backspace) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"sa\b");
    BOOST_CHECK_EQUAL(u8"s", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_T8) {
    TKEngine e;
    e.reset();
    feedKeys(e, u8"ahy");
    BOOST_CHECK_EQUAL(u8"a̍h", e.getBuffer());
}

BOOST_AUTO_TEST_SUITE_END();

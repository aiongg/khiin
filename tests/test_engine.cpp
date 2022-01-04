// libtaikey_test.cpp : Source file for your target.
//

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <vector>

#include "libtaikey.h"

using namespace std;
using namespace TaiKey;
namespace fs = std::filesystem;

typedef pair<string, string> test_w;

struct EngineFx {
    EngineFx() : e(Engine("./")) {
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
    ~EngineFx() { words.clear(); }
    Engine e;
    void feedKeys(const char *keys) {
        e.reset();
        for (auto c : std::string(keys)) {
            e.onKeyDown(c);
        }
    } 
    vector<test_w> words;
};

BOOST_FIXTURE_TEST_SUITE(TestEngine, EngineFx);

BOOST_AUTO_TEST_CASE(Engine_Empty) {
    e.reset();
    BOOST_CHECK_EQUAL(u8"", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Buffer) {
    feedKeys(u8"a");
    BOOST_CHECK_EQUAL(u8"a", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Tone) {
    feedKeys(u8"as");
    BOOST_CHECK_EQUAL(u8"á", e.getBuffer());
    feedKeys(u8"af");
    BOOST_CHECK_EQUAL(u8"à", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_TonePlacement) {
    feedKeys(u8"uis");
    BOOST_CHECK_EQUAL(u8"úi", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Failure, *boost::unit_test::expected_failures(1)) {
    feedKeys(u8"a");
    BOOST_CHECK_EQUAL(u8"b", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Words) {
    vector<string> expected;
    vector<string> actual;

    for (test_w word : words) {
        e.reset();
        feedKeys(word.first.c_str());
        expected.push_back(word.second);
        actual.push_back(e.getBuffer());
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                  expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(Engine_OU) {
    feedKeys(u8"ou");
    BOOST_CHECK_EQUAL(u8"o͘", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Nasal) {
    feedKeys(u8"ann");
    BOOST_CHECK_EQUAL(u8"a\u207f", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_OU_Tone) {
    e.reset();
    feedKeys(u8"ous");
    BOOST_CHECK_EQUAL(u8"ó͘", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Nasal_Tone) {
    feedKeys(u8"annf");
    BOOST_CHECK_EQUAL(u8"à\u207f", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_OU_ToneChange) {
    feedKeys(u8"ousf");
    BOOST_CHECK_EQUAL(u8"ò͘", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_Backspace) {
    feedKeys(u8"sa\b");
    BOOST_CHECK_EQUAL(u8"s", e.getBuffer());
}

BOOST_AUTO_TEST_CASE(Engine_T8) {
    feedKeys(u8"ahy");
    BOOST_CHECK_EQUAL(u8"a̍h", e.getBuffer());
}

BOOST_AUTO_TEST_SUITE_END();

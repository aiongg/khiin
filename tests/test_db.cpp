#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include "db.h"

using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(Database)

BOOST_AUTO_TEST_CASE(select_syllable_list) {
    TKDB db("taikey.db");
    auto res = db.selectSyllableList();
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(select_word_list) {
    TKDB db("taikey.db");
    auto res = db.selectTrieWordlist();
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(select_dictionary_by_ascii) {
    TKDB db("taikey.db");
    auto res = db.selectDictionaryRowsByAscii("a");
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END()

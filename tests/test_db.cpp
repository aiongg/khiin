#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp>

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
    auto res = DictRows();
    db.selectDictionaryRowsByAscii("a", res);
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(select_by_ascii_list) {
    TKDB db("taikey.db");
    auto v = std::vector<std::string>();
    v.push_back("ong");
    v.push_back("ong5");
    auto res = DictRows();
    db.selectDictionaryRowsByAscii(v, res);
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(update_gram_counts) {
    TKDB db("taikey.db");
    auto v = std::vector<std::string>();
    auto text = u8"囝仔 王 子 是 無 國 界 个 文 學 經 典";

    boost::split(v, text, boost::is_any_of(" "));

    auto res = db.updateGramCounts(v);
    BOOST_TEST(res == 0);
}

BOOST_AUTO_TEST_SUITE_END()

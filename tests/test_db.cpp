#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include "db.h"

using namespace taikey;

static const auto DB_FILE = "taikey.db";

struct DbFx {
    DbFx() { db = new TKDB(DB_FILE); }
    ~DbFx() { delete db; }
    TKDB *db = nullptr;
};

BOOST_FIXTURE_TEST_SUITE(Database, DbFx)

BOOST_AUTO_TEST_CASE(select_syllable_list) {
    auto res = db->selectSyllableList();
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(select_word_list) {
    auto res = db->selectTrieWordlist();
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(select_dictionary_by_ascii) {
    auto res = DictRows();
    db->selectDictionaryRowsByAscii("a", res);
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(select_by_ascii_list) {
    auto v = std::vector<std::string>();
    v.push_back("ong");
    v.push_back("ong5");
    auto res = DictRows();
    db->selectDictionaryRowsByAscii(v, res);
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(update_gram_counts) {
    auto v = std::vector<std::string>();
    auto text = u8"囝仔 王 子 是 無 國 界 个 文 學 經 典";

    boost::split(v, text, boost::is_any_of(" "));

    auto res = db->updateGramCounts(v);
    BOOST_TEST(res == 23);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(DummyDb)

BOOST_AUTO_TEST_CASE(dummy_db) {
    auto dummy = new TKDB();
    auto words = dummy->selectTrieWordlist();
    BOOST_TEST(words.size() == 1);
    delete dummy;
}

BOOST_AUTO_TEST_SUITE_END()

#include <gtest/gtest.h>

#include <boost/algorithm/string.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include "db.h"

namespace khiin::engine {
namespace {

static const auto DB_FILE = "taikey.db";

class DbFx : public ::testing::Test {
  protected:
    void SetUp() override {
        db = new TKDB(DB_FILE);
    }
    ~DbFx() {
        delete db;
    }
    TKDB *db = nullptr;
};

TEST_F(DbFx, select_syllable_list) {
    auto res = db->selectSyllableList();
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, select_word_list) {
    auto res = db->selectTrieWordlist();
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, select_dictionary_by_ascii) {
    auto res = DictRows();
    db->selectDictionaryRowsByAscii("a", res);
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, select_by_ascii_list) {
    auto v = std::vector<std::string>();
    v.push_back("ong");
    v.push_back("ong5");
    auto res = DictRows();
    db->selectDictionaryRowsByAscii(v, res);
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, update_gram_counts) {
    auto v = std::vector<std::string>();
    auto text = u8"囝仔 王 子 是 無 國 界 个 文 學 經 典";

    boost::split(v, text, boost::is_any_of(" "));

    auto res = db->updateGramCounts(v);
    EXPECT_EQ(res, 23);
}

TEST(Database, DummyDb) {
    auto dummy = new TKDB();
    auto words = dummy->selectTrieWordlist();
    EXPECT_EQ(words.size(), 1);
    delete dummy;
}

} // namespace
} // namespace khiin::engine

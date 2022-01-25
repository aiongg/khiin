#include <gtest/gtest.h>

#include <boost/algorithm/string.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Database.h"

namespace khiin::engine {
namespace {

static const auto DB_FILE = "taikey.db";

class DbFx : public ::testing::Test {
  protected:
    void SetUp() override {
        db = new Database(DB_FILE);
    }
    ~DbFx() {
        delete db;
    }
    Database *db = nullptr;
};

TEST_F(DbFx, select_syllable_list) {
    auto res = db->GetSyllableList();
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, select_word_list) {
    auto res = db->GetTrieWordlist();
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, select_dictionary_by_ascii) {
    auto res = DictRows();
    db->SearchDictionaryByAscii("a", res);
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, select_by_ascii_list) {
    auto v = std::vector<std::string>();
    v.push_back("ong");
    v.push_back("ong5");
    auto res = DictRows();
    db->SearchDictionaryByAscii(v, res);
    EXPECT_GT(res.size(), 0);
}

TEST_F(DbFx, update_gram_counts) {
    auto v = std::vector<std::string>();
    auto text = u8"囝仔 王 子 是 無 國 界 个 文 學 經 典";

    boost::split(v, text, boost::is_any_of(" "));

    auto res = db->IncrementNGramCounts(v);
    EXPECT_EQ(res, 23);
}

TEST(Database, DummyDb) {
    auto dummy = new Database();
    auto words = dummy->GetTrieWordlist();
    EXPECT_EQ(words.size(), 1);
    delete dummy;
}

} // namespace
} // namespace khiin::engine
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/algorithm/string.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Database.h"

namespace khiin::engine {
namespace {
using ::testing::Contains;

static const auto kDatabaseFilename = "khiin_test.db";

class DatabaseTest : public ::testing::Test {
  protected:
    void SetUp() override {
        db = Database::Connect(kDatabaseFilename);
    }
    void TearDown() override {
        db->ClearNGramsData();
        delete db;
    }
    Database *db = nullptr;
};

TEST_F(DatabaseTest, RecordUnigrams) {
    std::vector<std::string> grams = {"a", "b", "c", "b"};
    db->RecordUnigrams(grams);
    EXPECT_EQ(db->UnigramCount("a"), 1);
    EXPECT_EQ(db->UnigramCount("b"), 2);
    EXPECT_EQ(db->UnigramCount("c"), 1);
    EXPECT_EQ(db->UnigramCount("d"), 0);
    grams = {"a"};
    db->RecordUnigrams(grams);
    EXPECT_EQ(db->UnigramCount("a"), 2);
}

TEST_F(DatabaseTest, RecordBigrams) {
    std::vector<Database::Bigram> grams = {{"a", "b"}, {"b", "c"}};
    db->RecordBigrams(grams);
    EXPECT_EQ(db->BigramCount({"a", "b"}), 1);
    EXPECT_EQ(db->BigramCount({"b", "c"}), 1);
    EXPECT_EQ(db->BigramCount({"c", "d"}), 0);
    grams = {{"b", "c"}, {"c", "d"}};
    db->RecordBigrams(grams);
    EXPECT_EQ(db->BigramCount({"b", "c"}), 2);
    EXPECT_EQ(db->BigramCount({"c", "d"}), 1);
}

TEST_F(DatabaseTest, GetUnigramCounts) {
    std::vector<std::string> grams = {"a", "b", "c", "b"};
    db->RecordUnigrams(grams);
    auto result = db->UnigramCounts(grams);
    std::sort(result.begin(), result.end(), [&](auto a, auto b) {
        return a.value < b.value;
    });
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].value, "a");
    EXPECT_EQ(result[0].count, 1);
    EXPECT_EQ(result[1].value, "b");
    EXPECT_EQ(result[1].count, 2);
    EXPECT_EQ(result[2].value, "c");
    EXPECT_EQ(result[2].count, 1);
}

TEST_F(DatabaseTest, GetBigramCounts) {
    std::vector<Database::Bigram> grams = {{"a", "b"}};
    db->RecordBigrams(grams);
    auto result = db->BigramCounts("a", {"b"});
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].value, "b");
    EXPECT_EQ(result[0].count, 1);
}

TEST_F(DatabaseTest, select_syllable_list) {
    // auto res = db->GetSyllableList();
    // EXPECT_GT(res.size(), 0);
}

TEST_F(DatabaseTest, select_word_list) {
    // auto res = db->GetTrieWordlist();
    // EXPECT_GT(res.size(), 0);
}

TEST_F(DatabaseTest, select_dictionary_by_ascii) {
    // auto res = DictRows();
    // db->SearchDictionaryByAscii("a", res);
    // EXPECT_GT(res.size(), 0);
}

TEST_F(DatabaseTest, select_by_ascii_list) {
    auto v = std::vector<std::string>();
    v.push_back("ong");
    v.push_back("ong5");
    // auto res = DictRows();
    // db->SearchDictionaryByAscii(v, res);
    // EXPECT_GT(res.size(), 0);
}

TEST_F(DatabaseTest, update_gram_counts) {
    auto v = std::vector<std::string>();
    auto text = u8"囝仔 王 子 是 無 國 界 个 文 學 經 典";

    boost::split(v, text, boost::is_any_of(" "));

    // auto res = db->IncrementNGramCounts(v);
    // EXPECT_EQ(res, 23);
}

TEST(DummyDatabase, Exists) {
    auto dummy = Database::TestDb();
    // auto words = dummy->GetTrieWordlist();
    // EXPECT_EQ(words.size(), 1);
    delete dummy;
}

} // namespace
} // namespace khiin::engine

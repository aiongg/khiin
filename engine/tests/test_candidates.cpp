#include <gtest/gtest.h>

#include <boost/log/trivial.hpp>

#include "CandidateFinder.h"

namespace khiin::engine {
namespace {

const std::string DB_FILE = "taikey.db";

class CandidateFx : public ::testing::Test {
  protected:
    void SetUp() override {
        db = new Database(DB_FILE);
        auto sylList = db->GetSyllableList();
        splitter = new Splitter(sylList), trie = new Trie(db->GetTrieWordlist(), sylList);
        cf = CandidateFinder::Create(db, splitter, trie);
    }
    ~CandidateFx() {
        delete db;
        delete splitter;
        delete trie;
        delete cf;
    }

    Candidate fuzzyPrimary(std::string search) {
        return cf->findPrimaryCandidate(search, "", true);
    }

    Database *db = nullptr;
    Splitter *splitter = nullptr;
    Trie *trie = nullptr;
    CandidateFinder *cf = nullptr;
};

TEST_F(CandidateFx, load_candidate_finder) {
    auto res = fuzzyPrimary("khiameng");
    EXPECT_GT(res.size(), 0);
}

TEST_F(CandidateFx, find_candidates) {
    auto res = fuzzyPrimary("khiameng");
    EXPECT_GT(res.size(), 0);
}

TEST_F(CandidateFx, check_empty_search) {
    auto res = fuzzyPrimary("");
    EXPECT_EQ(res.size(), 0);
}

TEST_F(CandidateFx, find_long_string) {
    auto test = "chetioittengsisekaisiongchanephahjihoat";
    auto res = fuzzyPrimary(test);

    auto v = std::string();

    for (auto &chunk : res) {
        v += chunk.token.output;
    }

    BOOST_LOG_TRIVIAL(debug) << "Input string: " << test;
    BOOST_LOG_TRIVIAL(debug) << "By default: " << v;

    EXPECT_GT(res.size(), 0);

    BOOST_LOG_TRIVIAL(debug) << "Inserting training data...";

    auto grams = string_vector{u8"賛", u8"个", u8"打", u8"字", u8"法"};
    db->IncrementNGramCounts(grams);

    res = fuzzyPrimary(test);

    v.clear();

    for (auto &chunk : res) {
        v += chunk.token.output;
    }

    BOOST_LOG_TRIVIAL(debug) << "Re-running candidate selection: " << v;
}

} // namespace
} // namespace khiin::engine
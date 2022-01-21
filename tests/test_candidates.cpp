#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/log/trivial.hpp>

#include "candidates.h"

namespace taikey::CandidateTest {

const std::string DB_FILE = "taikey.db";

struct CandidateFx {
    CandidateFx() {
        db = new TKDB(DB_FILE);
        auto sylList = db->selectSyllableList();
        splitter = new Splitter(sylList),
        trie = new Trie(db->selectTrieWordlist(), sylList);
        cf = new CandidateFinder(db, splitter, trie);
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

    TKDB *db = nullptr;
    Splitter *splitter = nullptr;
    Trie *trie = nullptr;
    CandidateFinder *cf = nullptr;
};

BOOST_FIXTURE_TEST_SUITE(CandidateTest, CandidateFx)

BOOST_AUTO_TEST_CASE(load_candidate_finder) {
    auto res = fuzzyPrimary("khiameng");
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(find_candidates) {
    auto res = fuzzyPrimary("khiameng");
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(check_empty_search) {
    auto res = fuzzyPrimary("");
    BOOST_TEST(res.size() == 0);
}

BOOST_AUTO_TEST_CASE(find_long_string) {
    auto test = "chetioittengsisekaisiongchanephahjihoat";
    auto res = fuzzyPrimary(test);

    auto v = std::string();

    for (auto &chunk : res) {
        v += chunk.token.output;
    }

    BOOST_LOG_TRIVIAL(debug) << "Input string: " << test;
    BOOST_LOG_TRIVIAL(debug) << "By default: " << v;

    BOOST_TEST(res.size() > 0);

    BOOST_LOG_TRIVIAL(debug) << "Inserting training data...";

    auto grams = VStr{u8"賛", u8"个", u8"打", u8"字", u8"法"};
    db->updateGramCounts(grams);

    res = fuzzyPrimary(test);

    v.clear();

    for (auto &chunk : res) {
        v += chunk.token.output;
    }

    BOOST_LOG_TRIVIAL(debug) << "Re-running candidate selection: " << v;
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace TaiKey::CandidateTest
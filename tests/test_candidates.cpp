#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/log/trivial.hpp>

#include "candidates.h"

namespace TaiKey::CandidateTest {

const std::string DB_FILE = "taikey.db";

struct Fx {
    Fx()
        : db(DB_FILE), splitter(db.selectSyllableList()),
          trie(db.selectTrieWordlist(), db.selectSyllableList()),
          cf(db, splitter, trie) {}
    ~Fx() {}

    TKDB db;
    Splitter splitter;
    Trie trie;
    CandidateFinder cf;
};

BOOST_FIXTURE_TEST_SUITE(CandidateTest, Fx)

BOOST_AUTO_TEST_CASE(load_candidate_finder) {
    auto res = cf.findPrimaryCandidate("khiameng", true, "");
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(find_candidates) {
    auto res = cf.findCandidates("khiameng", true, "");
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(check_empty_search) {
    auto res = cf.findPrimaryCandidate("", true, "");
    BOOST_TEST(res.size() == 0);
}

BOOST_AUTO_TEST_CASE(find_long_string) {
    auto test = "chetioittengsisekaisiongchanephahjihoat";
    auto res = cf.findPrimaryCandidate(test, true, "");

    auto v = std::string();

    for (const auto &c : res) {
        v += c.output;
    }
    BOOST_LOG_TRIVIAL(debug) << "Input string: " << test;
    BOOST_LOG_TRIVIAL(debug) << "By default: " << v;

    BOOST_TEST(res.size() > 0);

    BOOST_LOG_TRIVIAL(debug)
        << "Inserting training data...";

    auto grams = VStr{u8"賛", u8"个", u8"打", u8"字", u8"法"}; 
    db.updateGramCounts(grams);

    res = cf.findPrimaryCandidate(test, true, "");

    v.clear();

    for (const auto &c : res) {
        v += c.output;
    }

    BOOST_LOG_TRIVIAL(debug) << "Re-running candidate selection: " << v;
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace TaiKey::CandidateTest
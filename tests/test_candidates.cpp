#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/log/trivial.hpp>

#include "candidates.h"

namespace TaiKey::CandidateTest {

struct Fx {
    Fx()
        : db("taikey.db"), splitter(db.selectSyllableList()),
          trie(db.selectTrieWordlist()), cf(db, splitter, trie) {}
    ~Fx() {}

    TKDB db;
    Splitter splitter;
    Trie trie;
    CandidateFinder cf;
};

BOOST_FIXTURE_TEST_SUITE(CandidateTest, Fx)

BOOST_AUTO_TEST_CASE(load_candidate_finder) {
    auto res = CandidateList();
    cf.findCandidates("khiameng", true, "", res);
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_CASE(check_empty_search) {
    auto res = CandidateList();
    cf.findCandidates("", true, "", res);
    BOOST_TEST(res.size() == 0);
}

BOOST_AUTO_TEST_CASE(find_long_string) {
    auto test = "chetioittengsisekaisiongchanephahjihoat";
    auto res = CandidateList();
    cf.findCandidates(test, true, "", res);

    auto v = std::string();

    for (auto c : res) {
        v += c.output;
    }
    BOOST_LOG_TRIVIAL(debug) << "Input string: " << test;
    BOOST_LOG_TRIVIAL(debug) << "By default: " << v;

    BOOST_TEST(res.size() > 0);

    BOOST_LOG_TRIVIAL(debug)
        << "Inserting training data...";

    auto grams = VStr{u8"賛", u8"个", u8"打", u8"字", u8"法"}; 
    db.updateGramCounts(grams);

    cf.findCandidates(test, true, "", res);

    v.clear();

    for (auto c : res) {
        v += c.output;
    }

    BOOST_LOG_TRIVIAL(debug) << "Re-running candidate selection: " << v;
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace TaiKey::CandidateTest
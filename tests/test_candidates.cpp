#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "candidates.h"

using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(Candidates);

BOOST_AUTO_TEST_CASE(load_candidate_finder) {
    auto db = TKDB("taikey.db");
    auto syllables = db.selectSyllableList();
    auto splitter = Splitter(syllables);
    auto dictionary = db.selectTrieWordlist();
    auto trie = Trie(dictionary);

    auto cf = CandidateFinder(db, splitter, trie);

    auto res = CandidateList();
    cf.findCandidates("khiameng", true, res);
    BOOST_TEST(res.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END();

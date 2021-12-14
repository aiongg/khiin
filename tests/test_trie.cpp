#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "trie.h"

using namespace std;
using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(TrieTest);

BOOST_AUTO_TEST_CASE(Trie_Create) {
    TNode *root = new TNode;
    BOOST_CHECK_EQUAL(root->hasChildren(), false);
}

BOOST_AUTO_TEST_CASE(Trie_Insert) {
    word_wt w1(u8"貓", 10);
    TNode *root = new TNode;
    root->insert(u8"niau", w1);
    BOOST_CHECK(root->children['n']
                    ->children['i']
                    ->children['a']
                    ->children['u']
                    ->leaves[0]
                    .first == u8"貓");
}

BOOST_AUTO_TEST_CASE(Trie_Search) {
    word_wt w1(u8"貓", 10);
    TNode *root = new TNode;
    root->insert(u8"niau", w1);
    word_wt_v res = root->searchExact(u8"niau");
    
    BOOST_CHECK_EQUAL(res.size(), 1);
    BOOST_CHECK_EQUAL(res[0].first, u8"貓");
}

BOOST_AUTO_TEST_CASE(Trie_Remove) {
    word_wt w1(u8"貓", 10);
    TNode *root = new TNode;
    root->insert(u8"niau", w1);
    root->remove(u8"niau", u8"貓");
    BOOST_CHECK_EQUAL(root->children.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END();

#include "candidates.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

CandidateFinder::CandidateFinder(TKDB &db) : db_(db) {
    auto syllables = db_.selectSyllableList();
    splitter_ = Splitter(syllables);

    auto dictionary = db_.selectTrieWordlist();
    trie_ = Trie(dictionary);
}

} // namespace TaiKey
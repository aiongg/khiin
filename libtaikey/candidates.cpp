#include <optional>

#include "candidates.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

using namespace std::literals::string_literals;

// CandidateFinder::CandidateFinder(TKDB &db) : db_(db) {
//    auto syllables = db_.selectSyllableList();
//    splitter_ = Splitter(syllables);
//
//    auto dictionary = db_.selectTrieWordlist();
//    trie_ = Trie(dictionary);
//}

CandidateFinder::CandidateFinder(TKDB &db, Splitter &splitter, Trie &trie)
    : db_(db), splitter_(splitter), trie_(trie) {}

auto CandidateFinder::findCandidates(std::string query, bool toneless,
                                     CandidateList &results) -> void {
    results.clear();
    // Split query with spaces
    // Get longest Word match for query in Trie (optional tones)
    // Take all matches that align with our given syllable splits (optional
    // tones) Check db 2-grams & 1-grams, else check dictionary weights repeat
    // from after the longest match
    auto syllables = VStr();
    splitter_.split(query, syllables);

    auto ptr = query.begin();
    auto prevBestGram = "#"s;
    auto currWords = VStr();
    auto rows = DictRows();

    while (ptr != query.end()) {
        auto remainder = std::string(ptr, query.end());

        trie_.getAllWords(query, toneless, currWords);
        db_.selectDictionaryRowsByAsciiWithUnigram(currWords, rows);
        ptr++;
    }

    return;
}

} // namespace TaiKey
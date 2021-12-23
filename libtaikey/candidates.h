#pragma once

#include <string>

#include "db.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

enum class CColor {
    Normal,
    Red,
    Gray,
};

struct Candidate {
    std::string ascii;
    std::string output;
    std::string hint;
    CColor color = CColor::Normal;
};

using CandidateList = std::vector<Candidate>;

// buffer
// auto momomo = candidates.find(rawBuffer);

// struct Candidate {
//    std::string input;
//    std::string output;
//};

class CandidateFinder {
  public:
    CandidateFinder(TKDB &db);
    CandidateFinder(TKDB &db, Splitter &splitter, Trie &trie);
    auto findCandidates(std::string query, bool toneless,
                        std::string prevBestGram, CandidateList &results)
        -> void;

  private:
    auto findBestCandidateByBigram_(std::string lgram, int lgramCount,
                                    const CandidateRows &rgrams) -> size_t;

    auto findBestCandidateBySplitter_(std::string input, bool toneless)
        -> CandidateRow;

    TKDB &db_;
    Splitter &splitter_;
    Trie &trie_;
};

} // namespace TaiKey
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

// buffer
// auto momomo = candidates.find(rawBuffer);

// struct Candidate {
//    std::string input;
//    std::string output;
//};

class CandidateFinder {
  public:
    CandidateFinder(TKDB &db, Splitter &splitter, Trie &trie);

    // AltCandidates is a list of <candidate, length> pairs, with length
    // corresponding to the number of characters consumed in the input string
    auto findCandidates(std::string input, bool toneless, std::string lgram)
        -> Candidates;
    auto findPrimaryCandidate(std::string query, bool toneless,
                              std::string lgram) -> Candidates;
    auto findPrimaryCandidate(std::string query, bool toneless,
                              Candidate lgram) -> Candidates;

  private:
    auto sortCandidatesByBigram_(std::string lgram, int lgramCount,
                                 Candidates &rgrams);
    auto findBestCandidateByBigram_(std::string lgram, int lgramCount,
                                    const Candidates &rgrams) -> size_t;

    auto findBestCandidateBySplitter_(std::string input, bool toneless)
        -> Candidate;

    TKDB &db_;
    Splitter &splitter_;
    Trie &trie_;
};

} // namespace TaiKey
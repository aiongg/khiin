#pragma once

#include <string>

#include "db.h"
#include "splitter.h"
#include "trie.h"

namespace TaiKey {

enum class CColor {
    Normal,
    Red,
    Gray,
};

class CandidateFinder {
  public:
    CandidateFinder(TKDB *db, Splitter *splitter, Trie *trie);

    auto findCandidates(std::string input, bool toneless, Candidate lgram)
        -> Candidates;
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

    TKDB *db;
    Splitter *splitter;
    Trie *trie;
};

} // namespace TaiKey
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
    std::string lomaji;
    std::string cand;
    std::string hint;
    CColor color = CColor::Normal;
};

using CandidateList = std::vector<Candidate>;

// buffer
// auto momomo = candidates.find(rawBuffer);

//struct Candidate {
//    std::string input;
//    std::string output;
//};

class CandidateFinder {
  public:
    CandidateFinder(TKDB &db);
    CandidateFinder(TKDB &db, Splitter &splitter, Trie &trie);
    auto findCandidates(std::string query, bool toneless,
                        CandidateList &results) -> void;

  private:
    TKDB &db_;
    Splitter &splitter_;
    Trie &trie_;
};

} // namespace TaiKey
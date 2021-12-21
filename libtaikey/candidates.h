#pragma once

#include <string>

#include "db.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

struct Candidate {
    std::string lomaji;
    std::string cand;
    std::string hint;
};

class CandidateFinder {
  public:
    CandidateFinder(TKDB &db);

  private:
    TKDB &db_;
    Splitter splitter_;
    Trie trie_;
};

} // namespace TaiKey
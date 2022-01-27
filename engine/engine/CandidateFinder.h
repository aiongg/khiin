#pragma once

#include <string>
#include "Database.h"
#include "Splitter.h"
#include "Trie.h"

namespace khiin::engine {

struct CandidateChunk {
    CandidateChunk(std::string raw);
    CandidateChunk(std::string raw, Token token);

    std::string raw;
    Token token;
};

using Candidate = std::vector<CandidateChunk>;
using Candidates = std::vector<Candidate>;

class CandidateFinder {
  public:
    static CandidateFinder *Create(Database *database, Splitter *splitter, Trie *trie);
    virtual Candidates findCandidates(std::string input, std::string lgram, bool toneless) = 0;
    virtual Candidate findPrimaryCandidate(std::string_view input, std::string lgram, bool fuzzy) = 0;
};

} // namespace khiin::engine

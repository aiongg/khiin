#pragma once

#include "Database.h"
#include "Splitter.h"
#include "Trie.h"
#include <string>

namespace khiin::engine {

class Engine;

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
    static CandidateFinder *Create(Engine *engine);
    virtual Candidates findCandidates(std::string input, std::string lgram, bool toneless) = 0;
    virtual Candidate findPrimaryCandidate(std::string_view input, std::string lgram, bool fuzzy) = 0;

    virtual void FindBestCandidate(std::string_view input, std::string_view lgram, int &candidate_id, size_t &consumed) = 0;
};

} // namespace khiin::engine

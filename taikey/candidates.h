#pragma once

#include <optional>
#include <string>

#include <c9/zip.h>

#include "db.h"
#include "splitter.h"
#include "trie.h"

namespace taikey {

enum class CColor {
    Normal,
    Red,
    Gray,
};


struct CandidateChunk {
    std::string raw;
    Token token;
    CandidateChunk(std::string raw);
    CandidateChunk(std::string raw, Token token);
};

using Candidate = std::vector<CandidateChunk>;
using Candidates = std::vector<Candidate>;

auto oneChunkCandidate(std::string &&raw, Token &&token);

// struct Candidate {
//    std::vector<std::string> inputs;
//    std::vector<std::optional<Token>> tokens;
//
//    Candidate(){};
//    Candidate(std::string &&input, Token &&token) {
//        inputs.push_back(std::move(input));
//        tokens.push_back(std::optional(std::move(token)));
//    }
//
//    auto empty() {
//        assert(inputs.empty() == tokens.empty());
//        return inputs.empty();
//    }
//
//    auto push_back(std::string &&input, std::optional<Token> &&token) -> void
//    {
//        inputs.push_back(std::move(input));
//        tokens.push_back(std::move(token));
//    }
//
//    auto size() {
//        assert(inputs.size() == tokens.size());
//        return inputs.size();
//    }
//
//    auto zip() const { return c9::zip(inputs, tokens); }
//};

class CandidateFinder {
  public:
    CandidateFinder(TKDB *db, Splitter *splitter, Trie *trie);

    auto findCandidates(std::string input, std::string lgram, bool toneless)
        -> Candidates;

    auto findPrimaryCandidate(std::string_view input, std::string lgram,
                              bool fuzzy) -> Candidate;

  private:
    auto sortTokensByBigram(std::string lgram, int lgramCount, Tokens &rgrams);

    auto bestTokenByBigram(std::string lgram, int lgramCount,
                           const Tokens &rgrams) -> size_t;

    auto findBestCandidateBySplitter_(std::string input, bool toneless)
        -> Token;

    auto handleNoTrieMatch(std::string_view::const_iterator &start,
                           const std::string_view::const_iterator &end,
                           Candidate &candidate, bool fuzzy) -> void;

    auto handleHyphens(std::string_view::const_iterator &start,
                       const std::string_view::const_iterator &end,
                       Candidate &candidate) -> bool;

    TKDB *db;
    Splitter *splitter;
    Trie *trie;
};

} // namespace TaiKey
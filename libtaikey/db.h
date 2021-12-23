#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "common.h"

namespace TaiKey {

using BigramWeightMap =
    std::unordered_map<std::pair<std::string, std::string>, int>;

struct DictionaryRow {
    int id;
    int chhan_id;
    std::string input;
    std::string output;
    int weight;
    int common;
    std::string hint;
};

struct CandidateRow {
    std::string ascii;
    std::string output;
    std::string hint;
    int common;
    int unigramN;
};

struct UnigramRow {
    std::string gram;
    int n;
};

using DictRows = std::vector<DictionaryRow>;
using CandidateRows = std::vector<CandidateRow>;
using BigramWeights = std::unordered_map<std::string, int>;

class TKDB {
  public:
    TKDB(std::string dbFilename);
    auto init() -> void;
    auto selectTrieWordlist() -> VStr;
    auto selectSyllableList() -> VStr;
    auto selectDictionaryRowsByAscii(std::string input, DictRows &results)
        -> void;
    auto selectDictionaryRowsByAscii(VStr inputs, DictRows &results) -> void;
    auto selectCandidatesFor(VStr queries, CandidateRows &results) -> void;
    auto selectBigramsFor(std::string lgram, VStr rgrams,
                          BigramWeights &results) -> void;
    auto getUnigramCount(std::string gram) -> int;
    auto updateGramCounts(VStr &grams) -> int;
    // auto selectBigrams(std::string lgram, VStr rgrams) -> BigramWeightMap;

  private:
    auto buildTrieLookupTable_() -> int;

    SQLite::Database db_;
    DictRows tableDictionary_;
};

} // namespace TaiKey

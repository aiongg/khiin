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
    int color;
    std::string hint;
};

struct Candidate {
    int dict_id;
    std::string ascii;
    std::string input;
    std::string output;
    std::string hint;
    int color = 0;
    int unigramN = 0;
    float bigramWt = 0.0f;
};

struct UnigramRow {
    std::string gram;
    int n;
};

using DictRows = std::vector<DictionaryRow>;
using Candidates = std::vector<Candidate>;
using BigramWeights = std::unordered_map<std::string, int>;

class TKDB {
  public:
    TKDB();
    TKDB(std::string dbFilename);
    auto init() -> void;
    auto selectTrieWordlist() -> VStr;
    auto selectSyllableList() -> VStr;
    auto selectDictionaryRowsByAscii(std::string input, DictRows &results)
        -> void;
    auto selectDictionaryRowsByAscii(VStr inputs, DictRows &results) -> void;
    auto selectCandidatesFor(VStr queries, Candidates &results) -> void;
    auto selectBigramsFor(std::string lgram, VStr rgrams,
                          BigramWeights &results) -> void;
    auto getUnigramCount(std::string gram) -> int;
    auto updateGramCounts(VStr &grams) -> int;
    // auto selectBigrams(std::string lgram, VStr rgrams) -> BigramWeightMap;

  private:
    auto buildTrieLookupTable_() -> int;

    SQLite::Database handle;
    DictRows tableDictionary;
};

} // namespace TaiKey

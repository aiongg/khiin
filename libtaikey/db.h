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
    int unigramN;
    int bigramN;
};

using DictRows = std::vector<DictionaryRow>;

class TKDB {
  public:
    TKDB(std::string dbFilename);
    auto init() -> void;
    auto selectTrieWordlist() -> VStr;
    auto selectSyllableList() -> VStr;
    auto selectDictionaryRowsByAscii(std::string input, DictRows &results)
        -> void;
    auto selectDictionaryRowsByAscii(VStr inputs, DictRows &results) -> void;
    auto selectDictionaryRowsByAsciiWithUnigram(VStr queries, DictRows &results)
        -> void;
    auto updateGramCounts(VStr &grams) -> int;
    //auto selectBigrams(std::string lgram, VStr rgrams) -> BigramWeightMap;

  private:
    auto buildTrieLookupTable_() -> int;

    SQLite::Database db_;
    DictRows tableDictionary_;
};

} // namespace TaiKey

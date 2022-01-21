#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "common.h"

namespace taikey {

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

struct Token {
    int dict_id = 0;
    std::string ascii;
    std::string input;
    std::string output;
    std::string hint;
    int color = 0;
    int unigramN = 0;
    float bigramWt = 0.0f;
    bool operator==(const Token &rhs) const {
        if (dict_id == 0 && rhs.dict_id == 0) {
            return ascii == rhs.ascii;
        } else {
            return dict_id == rhs.dict_id;
        }
    };
    bool operator!=(const Token &rhs) const { return !(*this == rhs); };
    void clear() {
        dict_id = 0;
        ascii.clear();
        input.clear();
        hint.clear();
        color = 0;
        unigramN = 0;
        bigramWt = 0.0f;
    }
    bool empty() const { return dict_id == 0; };
};

using Tokens = std::vector<Token>;

struct UnigramRow {
    std::string gram;
    int n;
};

using DictRows = std::vector<DictionaryRow>;
using BigramWeights = std::unordered_map<std::string, int>;

class TKDB {
  public:
    TKDB();
    TKDB(std::string dbFilename);
    auto getTokens(VStr queries, Tokens &results) -> void;
    auto init() -> void;
    auto selectTrieWordlist() -> VStr;
    auto selectSyllableList() -> VStr;
    auto selectDictionaryRowsByAscii(std::string input, DictRows &results)
        -> void;
    auto selectDictionaryRowsByAscii(VStr inputs, DictRows &results) -> void;
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

} // namespace taikey

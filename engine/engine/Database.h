#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include <SQLiteCpp/SQLiteCpp.h>

#include "common.h"
#include "Models.h"

namespace khiin::engine {

using BigramWeightMap = std::unordered_map<std::pair<std::string, std::string>, int>;

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
    bool operator!=(const Token &rhs) const {
        return !(*this == rhs);
    };
    void clear() {
        dict_id = 0;
        ascii.clear();
        input.clear();
        hint.clear();
        color = 0;
        unigramN = 0;
        bigramWt = 0.0f;
    }
    bool empty() const {
        return dict_id == 0;
    };
};

using Tokens = std::vector<Token>;

struct UnigramRow {
    std::string gram;
    int n;
};

using DictRows = std::vector<DictionaryRow>;
using BigramWeights = std::unordered_map<std::string, int>;

class Database {
  public:
    Database();
    Database(std::string dbFilename);
    void GetTokens(string_vector queries, Tokens &results);
    void Initialize();
    
    string_vector GetTrieWordlist();
    string_vector GetSyllableList();
    void SearchDictionaryByAscii(const std::string &input, DictRows &results);
    void SearchDictionaryByAscii(const string_vector &inputs, DictRows &results);
    void BigramsFor(const std::string &lgram, const string_vector &rgrams, BigramWeights &results);
    int UnigramCount(const std::string &gram);
    int IncrementNGramCounts(string_vector &grams);

    //void DictionaryWords(std::vector<std::string> &inputs);
    void LoadSyllables(std::vector<std::string> &syllables);
    void AllWordsByFreq(std::vector<DictionaryRow> &output);

    // auto selectBigrams(std::string lgram, string_vector rgrams) -> BigramWeightMap;

  private:
    int buildTrieLookupTable_();

    SQLite::Database handle;
    DictRows tableDictionary;
};

} // namespace khiin::engine

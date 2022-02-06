#pragma once

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Models.h"
#include "common.h"

namespace khiin::engine {

class Database {
  public:
    Database() = default;
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;
    ~Database() = default;

    static Database *TestDb();
    static Database *Connect(std::string db_filename);

    virtual DictionaryRow *HighestUnigramCount(std::vector<DictionaryRow *> const &grams) = 0;
    virtual DictionaryRow *HighestBigramCount(std::string const &lgram, std::vector<DictionaryRow *> const &rgrams) = 0;
    virtual void LoadSyllables(std::vector<std::string> &syllables) = 0;
    virtual void AllWordsByFreq(std::vector<DictionaryRow> &output) = 0;
};

} // namespace khiin::engine

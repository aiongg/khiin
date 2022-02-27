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

    virtual TaiToken *HighestUnigramCount(std::vector<TaiToken *> const &grams) = 0;
    virtual TaiToken *HighestBigramCount(std::string const &lgram, std::vector<TaiToken *> const &rgrams) = 0;
    virtual void LoadSyllables(std::vector<std::string> &syllables) = 0;
    virtual void AllWordsByFreq(std::vector<InputByFreq> &output) = 0;
    virtual void ConversionsByInputId(int input_id, std::vector<TaiToken> &conversions) = 0;
    virtual void LoadPunctuation(std::vector<Punctuation> &output) = 0;
};

} // namespace khiin::engine

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

//#include <SQLiteCpp/SQLiteCpp.h>

#include "data/Models.h"
#include "utils/common.h"

namespace khiin::engine {

class Database {
  public:
    using Bigram = std::pair<std::string, std::string>;

    Database() = default;
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;
    virtual ~Database() = 0;

    static std::unique_ptr<Database> TestDb();
    static std::unique_ptr<Database> Connect(std::string const &db_filename);

    virtual void ClearNGramsData() = 0;
    virtual void RecordUnigrams(std::vector<std::string> const &grams) = 0;
    virtual void RecordBigrams(std::vector<Bigram> const &grams) = 0;
    virtual int UnigramCount(std::string const &gram) = 0;
    virtual int BigramCount(Bigram const &gram) = 0;
    virtual std::vector<Gram> UnigramCounts(std::vector<std::string> const &grams) = 0;
    virtual std::vector<Gram> BigramCounts(std::string const &lgram, std::vector<std::string> const &rgrams) = 0;
    virtual TaiToken *HighestUnigramCount(std::vector<TaiToken *> const &grams) = 0;
    virtual TaiToken *HighestBigramCount(std::string const &lgram, std::vector<TaiToken *> const &rgrams) = 0;

    virtual void LoadSyllables(std::vector<std::string> &syllables) = 0;
    virtual void AllWordsByFreq(std::vector<InputByFreq> &output) = 0;
    virtual void ConversionsByInputId(int input_id, std::vector<TaiToken> &conversions) = 0;
    virtual void LoadPunctuation(std::vector<Punctuation> &output) = 0;
    virtual std::vector<Emoji> GetEmojis() = 0;
};

} // namespace khiin::engine

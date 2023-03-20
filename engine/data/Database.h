#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// #include <SQLiteCpp/SQLiteCpp.h>

#include "config/Config.h"
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

    virtual std::string CurrentConnection() = 0;

    virtual void ClearNGramsData() = 0;

    virtual void RecordUnigrams(std::vector<std::string> const &grams) = 0;

    virtual void RecordBigrams(std::vector<Bigram> const &grams) = 0;

    virtual void AddNGramsData(std::optional<std::string> const &lgram, std::vector<TaiToken> &tokens) = 0;

    virtual void LoadSyllables(std::vector<std::string> &syllables) = 0;

    virtual void AllWordsByFreq(std::vector<std::string> &output, InputType inputType) = 0;

    virtual void LoadConversions(std::vector<std::string> &inputs, InputType inputType,
                                 std::vector<TaiToken> &outputs) = 0;

    // virtual void ConversionsByInputId(int input_id, std::vector<TaiToken> &conversions) = 0;

    virtual void LoadPunctuation(std::vector<Punctuation> &output) = 0;

    virtual std::vector<Emoji> GetEmojis() = 0;
};

} // namespace khiin::engine

#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "common.h"
#include "utils.h"

namespace khiin::engine {

namespace db_tables {

namespace frequencies {
inline constexpr const char *id = "id";
inline constexpr const char *input = "input";
} // namespace frequencies

namespace conversions {
inline constexpr const char *id = "id";
inline constexpr const char *input_id = "input_id";
inline constexpr const char *output = "output";
inline constexpr const char *annotation = "annotation";
inline constexpr const char *category = "category";
inline constexpr const char *weight = "weight";
} // namespace conversions

namespace syllables {
inline constexpr const char *input = "input";
}

namespace unigram_freq {
inline constexpr const char *gram = "gram";
inline constexpr const char *count = "n";
} // namespace unigram_freq

namespace bigram_freq {
inline constexpr const char *lgram = "lgram";
inline constexpr const char *rgram = "rgram";
inline constexpr const char *count = "n";
} // namespace bigram_freq

namespace symbols {
inline constexpr const char *id = "id";
inline constexpr const char *input = "input";
inline constexpr const char *output = "output";
inline constexpr const char *annotation = "annotation";
} // namespace symbols

namespace emojis {
inline constexpr const char *category = "category";
inline constexpr const char *emoji = "emoji";
} // namespace emojis

} // namespace db_tables

class SQL {
  public:
    using Statement = SQLite::Statement;
    using DbHandle = SQLite::Database;
    static Statement SelectInputsByFreq(DbHandle &db);
    static Statement SelectSyllables(DbHandle &db);
    static Statement SelectConversions(DbHandle &db, int input_id);
    static Statement SelectConversions(DbHandle &db, std::vector<std::string *> const &inputs);
    static Statement SelectSymbols(DbHandle &db);
    static Statement SelectEmojis(DbHandle &db);

    // Ngrams
    static Statement SelectBestUnigram(DbHandle &db, std::vector<std::string *> const &grams);
    static Statement SelectBestBigram(DbHandle &db, std::string const &lgram,
                                              std::vector<std::string *> const &rgrams);
    static Statement SelectUnigrams(DbHandle &db, std::vector<std::string *> const &grams);
    static Statement SelectBigrams(DbHandle &db, std::string const &lgram,
                                           std::vector<std::string *> const &rgrams);
    static Statement SelectUnigramCount(DbHandle &db, std::string const &gram);
    static Statement SelectUnigramCounts(DbHandle &db, std::vector<std::string> const &grams);
    static Statement SelectBigramCount(DbHandle &db, std::string const &lgram,
                                               std::string const &rgram);
    static Statement SelectBigramCounts(DbHandle &db, std::string const &lgram,
                                                std::vector<std::string> const &rgrams);
    static Statement IncrementUnigrams(DbHandle &db, std::vector<std::string> const &grams);
    static Statement IncrementBigrams(DbHandle &db,
                                              std::vector<std::pair<std::string, std::string>> const &bigrams);
    static Statement DeleteUnigrams(DbHandle &db);
    static Statement DeleteBigrams(DbHandle &db);

    // DummyDb
    static Statement CreateDummyDb(DbHandle &db);
};

} // namespace khiin::engine

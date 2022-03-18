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
    static SQLite::Statement SelectInputsByFreq(SQLite::Database &db);
    static SQLite::Statement SelectSyllables(SQLite::Database &db);
    static SQLite::Statement SelectSymbols(SQLite::Database &db);
    static SQLite::Statement SelectEmojis(SQLite::Database &db);

    static SQLite::Statement SelectUnigramCounts(SQLite::Database &db, std::vector<std::string> const &grams);
    static SQLite::Statement SelectBestUnigram(SQLite::Database &db, std::vector<std::string *> const &grams);
    static SQLite::Statement SelectBestBigram(SQLite::Database &db, std::string const &lgram,
                                              std::vector<std::string *> const &rgrams);
    static SQLite::Statement SelectUnigramCount(SQLite::Database &db, std::string const &gram);
    static SQLite::Statement SelectBigramCount(SQLite::Database &db, std::string const &lgram,
                                               std::string const &rgram);
    static SQLite::Statement SelectUnigrams(SQLite::Database &db, std::vector<std::string *> const &grams);
    static SQLite::Statement SelectBigrams(SQLite::Database &db, std::string const &lgram,
                                           std::vector<std::string *> const &rgrams);
    static SQLite::Statement IncrementUnigrams(SQLite::Database &db, std::vector<std::string> const &grams);
    static SQLite::Statement IncrementBigrams(SQLite::Database &db,
                                              std::vector<std::pair<std::string, std::string>> const &bigrams);
    static SQLite::Statement DeleteUnigrams(SQLite::Database &db);
    static SQLite::Statement DeleteBigrams(SQLite::Database &db);
    static SQLite::Statement SelectConversions(SQLite::Database &db, int input_id);
    static SQLite::Statement SelectConversions(SQLite::Database &db, std::vector<std::string *> const &inputs);
    static SQLite::Statement CreateDummyDb(SQLite::Database &db);
};

} // namespace khiin::engine

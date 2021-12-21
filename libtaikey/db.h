#pragma once

#include <map>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

namespace TaiKey {

struct DictionaryRow {
    int id;
    int chhan_id;
    std::string input;
    std::string output;
    int weight;
    int common;
    std::string hint;
};

class TKDB {
    using VStr = std::vector<std::string>;
    using DictRowV = std::vector<DictionaryRow>;

  public:
    TKDB(std::string dbFilename);
    auto init() -> void;
    auto selectTrieWordlist() -> VStr;
    auto selectSyllableList() -> VStr;
    auto selectDictionaryRowsByAscii(std::string query) -> DictRowV;

  private:
    SQLite::Database db_;
    DictRowV tableDictionary_;

    auto buildTrieLookupTable_() -> int;
};

} // namespace TaiKey

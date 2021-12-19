#pragma once

#include <map>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

namespace TaiKey {

struct DictionaryRow {
    int id;
    int chhan_id;
    std::string lomaji;
    std::string hanji;
    int weight;
    bool common;
    std::string hint;
};

class TKDB {
  public:
    TKDB(std::string dbFilename);
    void init();

  private:
    SQLite::Database db_;
    std::vector<DictionaryRow> tableDictionary_;

    int buildTrieLookupTable_();
    std::vector<DictionaryRow> selectDictionaryByAscii(std::string query);
};

} // namespace TaiKey

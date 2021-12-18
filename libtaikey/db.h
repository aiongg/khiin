#pragma once

#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

namespace TaiKey {

class TKDB {
  private:
    std::string dbFilename_;
    SQLite::Database db_;
};

} // namespace TaiKey
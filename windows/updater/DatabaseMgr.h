#pragma once

#include <string>

namespace khiin::data {

class DatabaseMgr {
  public:
    static int Update(std::string const &old_db, std::string const &new_db);
};

} // namespace khiin::data

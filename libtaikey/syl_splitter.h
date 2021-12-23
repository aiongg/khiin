// cf https://stackoverflow.com/a/11642687

#pragma once

#include <set>
#include <unordered_map>

#include "errors.h"
#include "common.h"

namespace TaiKey {

class Splitter {
  public:
    Splitter();
    Splitter(const VStr &syllableList);
    auto canSplit(std::string input) -> bool;
    auto split(std::string input, std::vector<std::string> &result) -> RetVal;

  private:
    std::set<std::string> syllableSet_;
    std::unordered_map<std::string, float> costMap_;
    int maxWordLength_ = 0;
};

} // namespace TaiKey

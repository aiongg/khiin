// cf https://stackoverflow.com/a/11642687

#pragma once

#include <set>
#include <unordered_map>

#include "errors.h"
#include "common.h"

namespace khiin::engine {

class Splitter {
  public:
    Splitter();
    Splitter(const string_vector &syllableList);
    auto canSplit(std::string input) -> bool;
    auto split(std::string input) -> string_vector;
    auto split(std::string input, string_vector &result) -> RetVal;

  private:
    std::set<std::string> syllableSet_;
    std::unordered_map<std::string, float> costMap_;
    int maxWordLength_ = 0;
};

} // namespace khiin::engine
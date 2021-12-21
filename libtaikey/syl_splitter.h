// cf https://stackoverflow.com/a/11642687

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "errors.h"

namespace TaiKey {

class Splitter {
  public:
    Splitter();
    Splitter(const std::vector<std::string> &syllableList);
    auto split(std::string input, std::vector<std::string> &result) -> RetVal;

  private:
    std::unordered_map<std::string, float> costMap_;
    int maxWordLength_ = 0;
};

} // namespace TaiKey

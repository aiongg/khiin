// cf https://stackoverflow.com/a/11642687

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "errors.h"

namespace TaiKey {

class Splitter {
  public:
    Splitter(const std::vector<std::string> &syllableList);
    retval_t split(std::string input, std::vector<std::string> &result);

  private:
    std::unordered_map<std::string, float> costMap_;
    int maxWordLength_;
};

} // namespace TaiKey

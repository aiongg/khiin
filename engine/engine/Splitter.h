// cf https://stackoverflow.com/a/11642687

#pragma once

#include <set>
#include <unordered_map>

#include "common.h"
#include "errors.h"

namespace khiin::engine {
using WordCostMap = std::unordered_map<std::string, float>;

// Can determine whether a string may be split by the words contained in it,
// and can attempt to perform the best split (when the imported word list is
// sorted by frequency)
class Splitter {
  public:
    Splitter();
    Splitter(std::vector<std::string> const &words);
    size_t MaxSplitSize(std::string_view input) const;
    bool CanSplit(std::string_view input) const;
    void Split(std::string const &input, std::vector<std::string> &result) const;
    WordCostMap const &cost_map() const;

  private:
    std::unordered_set<std::string> m_word_set;
    WordCostMap m_cost_map;
    int m_max_word_length = 0;
};

} // namespace khiin::engine

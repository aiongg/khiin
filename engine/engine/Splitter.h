// cf https://stackoverflow.com/a/11642687

#pragma once

#include <set>
#include <unordered_map>

#include "common.h"
#include "errors.h"

namespace khiin::engine {
using SplitterCostMap = std::unordered_map<std::string, float>;

// Can determine whether a string may be split by the words contained in it,
// and can attempt to perform the best split (when the imported word list is
// sorted by frequency)
class Splitter {
  public:
    Splitter();
    Splitter(string_vector const &words);
    size_t MaxSplitSize(std::string const &input) const;
    bool CanSplit(std::string const &input) const;
    void Split(std::string const &input, string_vector &result) const;
    SplitterCostMap const &cost_map() const;

  private:
    std::unordered_set<std::string> m_word_set;
    SplitterCostMap m_cost_map;
    int m_max_word_length = 0;
};

} // namespace khiin::engine

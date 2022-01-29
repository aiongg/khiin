// cf https://stackoverflow.com/a/11642687

#pragma once

#include <set>
#include <unordered_map>

#include "common.h"
#include "errors.h"

namespace khiin::engine {

// Can determine whether a string may be split by the words contained in it,
// and can attempt to perform the best split (when the imported word list is
// sorted by frequency)
class Splitter {
  public:
    Splitter();
    Splitter(const string_vector &word_list);
    bool CanSplit(std::string const &input);
    void Split(std::string const &input, string_vector &result);

  private:
    std::set<std::string> m_word_set;
    std::unordered_map<std::string, float> m_cost_map;
    int m_max_word_length = 0;
};

} // namespace khiin::engine

#pragma once

#include <string>
#include <vector>

namespace khiin::engine {

class Engine;

class Dictionary {
  public:
    static Dictionary *Create(Engine *engine);
    virtual void BuildWordTrie() = 0;
    virtual void BuildSyllableTrie() = 0;
    virtual std::vector<std::string> WordSearch(std::string_view query) = 0;
};

} // namespace khiin::engine

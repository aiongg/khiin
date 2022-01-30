#pragma once

namespace khiin::engine {

class Engine;

class Dictionary {
  public:
    static Dictionary *Create(Engine *engine);
    virtual void BuildWordTrie() = 0;
};

} // namespace khiin::engine

#pragma once

namespace khiin::engine {

class Engine;
class KeyConfig;

class Dictionary {
  public:
    static Dictionary *Create(Engine *engine);
    virtual void SetKeyConfiguration(KeyConfig *key_configuration) = 0;
};

} // namespace khiin::engine

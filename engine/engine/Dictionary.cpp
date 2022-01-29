#include "Dictionary.h"

#include "Database.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "Trie.h"

namespace khiin::engine {

namespace {

class DictionaryImpl : public Dictionary {
  public:
    DictionaryImpl(Engine *engine) : engine(engine) {
        database = engine->database();
    }

    // Inherited via Dictionary
    virtual void SetKeyConfiguration(KeyConfig *key_configuration) override {}

  private:
    Engine *engine = nullptr;
    Database *database = nullptr;
    std::unordered_map<std::string, std::vector<int>> word_list;
    std::unique_ptr<Trie> word_trie = nullptr;
    std::unique_ptr<Trie> syllable_trie = nullptr;
};

} // namespace

Dictionary *Dictionary::Create(Engine *engine) {
    return new DictionaryImpl(engine);
}

} // namespace khiin::engine

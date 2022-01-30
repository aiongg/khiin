#include "Dictionary.h"

#include "Database.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "SyllableParser.h"
#include "Trie.h"

namespace khiin::engine {

namespace {

class DictionaryImpl : public Dictionary {
  public:
    DictionaryImpl(Engine *engine) : engine(engine) {}

    virtual void BuildWordTrie() override {
        word_trie = std::unique_ptr<Trie>(Trie::Create());
        auto parser = engine->syllable_parser();
        auto words = string_vector();

        engine->database()->DictionaryWords(words);

        for (auto &word : words) {
            auto keys = parser->GetMultisylInputSequences(word);
            for (auto &key : keys) {
                word_trie->Insert(key);
            }
        };
    }

    virtual void BuildSyllableTrie() override {
        syllable_trie = std::unique_ptr<Trie>(Trie::Create());
        auto syllables = string_vector();
        engine->database()->LoadSyllables(syllables);

        for (auto &syl : syllables) {
            syllable_trie->Insert(syl);
        }
    }

    virtual std::vector<std::string> WordSearch(std::string_view query) override {
        auto ret = std::vector<std::string>();
        word_trie->FindWords(query, ret);
        return ret;
    }

  private:
    Engine *engine = nullptr;
    std::unordered_map<std::string, std::vector<int>> word_list;
    std::unique_ptr<Trie> word_trie = nullptr;
    std::unique_ptr<Trie> syllable_trie = nullptr;

};

} // namespace

Dictionary *Dictionary::Create(Engine *engine) {
    auto dict = new DictionaryImpl(engine);
    dict->BuildWordTrie();
    dict->BuildSyllableTrie();
    return dict;
}

} // namespace khiin::engine

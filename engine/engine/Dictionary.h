#pragma once

#include <string>
#include <vector>

#include "Config.h"
#include "Models.h"

namespace khiin::engine {

class Engine;
class Splitter;
class Trie;

class Dictionary : public ConfigChangeListener {
  public:
    static Dictionary *Create(Engine *engine);
    virtual void Initialize() = 0;

    virtual std::vector<std::string> const &AllInputsByFreq() = 0;
    virtual std::vector<TaiToken *> WordSearch(std::string const &query) = 0;
    virtual std::vector<TaiToken *> Autocomplete(std::string const &query) = 0;
    virtual std::vector<TaiToken *> AllWordsFromStart(std::string const &query) = 0;
    virtual std::vector<std::vector<std::string>> Segment(std::string_view query, uint32_t limit) = 0;

    virtual bool StartsWithWord(std::string_view query) const = 0;
    virtual bool StartsWithSyllable(std::string_view query) const = 0;
    virtual bool IsWordPrefix(std::string_view query) const = 0;
    virtual bool IsSyllablePrefix(std::string_view query) const = 0;
    virtual bool IsWord(std::string_view query) const = 0;

    virtual std::vector<std::string> SearchPunctuation(std::string const &query) = 0;

    virtual Splitter *word_splitter() = 0;
    virtual Trie *word_trie() = 0;

    // Inherited via ConfigChangeListener
    virtual void OnConfigChanged(messages::AppConfig config) override = 0;
};

} // namespace khiin::engine

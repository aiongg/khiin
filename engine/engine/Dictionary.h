#pragma once

#include <string>
#include <vector>

#include "Config.h"
#include "Models.h"

namespace khiin::engine {

class Buffer;
class Config;
class Engine;
class Splitter;
class Trie;

struct TokenResult {
    TaiToken *token = nullptr;
    size_t input_size = 0;
};

class Dictionary : public ConfigChangeListener {
  public:
    static Dictionary *Create(Engine *engine);
    virtual void Initialize() = 0;

    virtual std::vector<std::string> const &AllInputsByFreq() = 0;
    virtual std::vector<TokenResult> WordSearch(std::string const &query) = 0;
    virtual std::vector<TokenResult> Autocomplete(std::string const &query) = 0;
    virtual std::vector<TokenResult> AllWordsFromStart(std::string const &query) = 0;
    virtual std::vector<std::vector<std::string>> Segment(std::string_view query, uint32_t limit) = 0;

    virtual bool StartsWithWord(std::string_view query) const = 0;
    virtual bool StartsWithSyllable(std::string_view query) const = 0;
    virtual bool IsWordPrefix(std::string_view query) const = 0;
    virtual bool IsSyllablePrefix(std::string_view query) const = 0;
    virtual bool IsWord(std::string_view query) const = 0;

    virtual std::vector<Punctuation> SearchPunctuation(std::string const &query) = 0;

    virtual void RecordNGrams(Buffer const &buffer) = 0;

    virtual Splitter *word_splitter() = 0;
    virtual Trie *word_trie() = 0;

    // Inherited via ConfigChangeListener
    virtual void OnConfigChanged(Config *config) override = 0;
};

} // namespace khiin::engine

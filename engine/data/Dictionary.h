#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Models.h"
#include "config/Config.h"

namespace khiin::engine {

class Buffer;
class Config;
class Engine;
class Splitter;
class Trie;

class Dictionary : public ConfigChangeListener {
   public:
    Dictionary() = default;
    Dictionary& operator=(Dictionary const&) = delete;
    Dictionary(Dictionary const&) = delete;
    virtual ~Dictionary() = 0;

    static std::unique_ptr<Dictionary> Create(Engine* engine);
    virtual void Initialize() = 0;
    virtual void Uninitialize() = 0;

    virtual std::vector<std::string> const& AllInputsByFreq() = 0;
    virtual std::vector<TaiToken> WordSearch(std::string const& query) = 0;
    virtual std::vector<TaiToken> Autocomplete(std::string const& query) = 0;
    virtual std::vector<TaiToken> AllWordsFromStart(std::string const& query) = 0;
    // virtual std::vector<TokenResult> UserWords(std::string const &query) = 0;

    virtual std::vector<std::vector<std::string>> Segment(
        std::string_view query,
        uint32_t limit) = 0;

    virtual bool StartsWithWord(std::string_view query) const = 0;
    virtual bool StartsWithSyllable(std::string_view query) const = 0;
    virtual bool IsWordPrefix(std::string_view query) const = 0;
    virtual bool IsSyllablePrefix(std::string_view query) const = 0;
    virtual bool IsWord(std::string_view query) const = 0;

    virtual std::vector<Punctuation> SearchPunctuation(std::string const& query) = 0;

    virtual void RecordNGrams(Buffer const& buffer) = 0;

    virtual Splitter* word_splitter() = 0;
    virtual Trie* word_trie() = 0;

    // Inherited via ConfigChangeListener
    void OnConfigChanged(Config* config) override = 0;
};

}  // namespace khiin::engine

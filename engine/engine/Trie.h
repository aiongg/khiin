#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "common.h"

namespace khiin::engine {

class Trie {
  public:
    //Trie();
    //Trie(const string_vector &keys);
    static Trie *Create();
    static Trie *Create(std::vector<std::string> const &words);

    virtual void Insert(std::string_view key) = 0;
    virtual bool Remove(std::string_view key) = 0;

    // If query matches any key from the start
    virtual bool StartsWithKey(std::string_view query) = 0;

    // If query is a key
    virtual bool HasKey(std::string_view query) = 0;

    // If query is a key or a key prefix
    virtual bool HasKeyOrPrefix(std::string_view query) = 0;

    virtual size_t LongestKeyOf(std::string_view query) = 0;

    virtual string_vector Autocomplete(std::string const &query, size_t limit = 0, size_t max_depth = 0) = 0;
    virtual void FindKeys(std::string_view query, bool fuzzy, string_vector &results) = 0;
    virtual void FindWords(std::string_view query, string_vector &results) = 0;
};

} // namespace khiin::engine

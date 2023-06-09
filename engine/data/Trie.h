#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "utils/common.h"

namespace khiin::engine {

class Trie {
  public:
    //Trie() = default;
    virtual ~Trie() = default;
    // Trie(const string_vector &keys);
    static std::unique_ptr<Trie> Create();

    virtual void Insert(std::vector<std::string> const &words) = 0;
    virtual void Insert(std::string_view key) = 0;
    virtual bool Remove(std::string_view key) = 0;

    // If query matches any key from the start
    virtual bool StartsWithKey(std::string_view query) = 0;

    virtual std::vector<std::vector<int>>
    Multisplit(std::string_view query, std::unordered_map<std::string, float> const &cost_map, uint32_t limit) = 0;

    // If query is a key
    virtual bool HasKey(std::string_view query) = 0;

    // If query is a key or a key prefix
    virtual bool HasKeyOrPrefix(std::string_view query) = 0;

    virtual size_t LongestKeyOf(std::string_view query) = 0;

    virtual std::vector<std::string> Autocomplete(std::string const &query, int limit = 0, int max_depth = 0) = 0;
    // virtual void FindKeys(std::string_view query, bool fuzzy, string_vector &results) = 0;
    virtual void FindKeys(std::string_view query, std::vector<std::string> &results) = 0;
};

} // namespace khiin::engine

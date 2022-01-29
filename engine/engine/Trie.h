#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "common.h"

namespace khiin::engine {

class Trie {
  public:
    Trie();
    Trie(const string_vector &keys);
    void Insert(std::string_view key);
    bool Remove(std::string_view key);
    bool ContainsWord(std::string_view query);
    bool ContainsPrefix(std::string_view query);
    string_vector Autocomplete(std::string const & query, size_t maxDepth = 0);
    //string_vector autocompleteTone(std::string query);
    void FindKeys(std::string_view query, bool fuzzy, string_vector &results);

  private:
    struct Node;
    using ChildrenType = std::unordered_map<char, std::unique_ptr<Node>>;
    struct Node {
        ChildrenType children;
        bool isEndOfWord = false;
        auto hasChild(char ch) -> bool;
    };
    Node root;

    // auto remove_(std::string key, int depth) -> bool;
    void insert_(Node &root, std::string_view key);
    Node *find_(Node &root, std::string_view query);
    void dfs_(Node *root, std::string const &query, std::string const &prefix, std::vector<std::string> &results,
              size_t maxDepth = 0);
};

} // namespace khiin::engine

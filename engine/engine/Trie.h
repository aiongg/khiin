#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "common.h"

namespace khiin::engine {

struct RecursiveMap {
    std::unordered_map<std::string, RecursiveMap> map;
};

class Trie {
  public:
    Trie();
    Trie(string_vector words, string_vector syllables);
    auto insert(std::string key) -> void;
    auto remove(std::string key) -> bool;
    auto containsWord(std::string query) -> bool;
    auto containsPrefix(std::string query) -> bool;
    auto containsSyllablePrefix(std::string query) -> bool;
    auto autocomplete(std::string query, size_t maxDepth = 0) -> string_vector;
    auto autocompleteTone(std::string query) -> string_vector;
    auto getAllWords(std::string query, bool isToneless) -> string_vector;
    auto getAllWords(std::string query, bool isToneless, string_vector &results) -> void;

    // TODO Can delete
    void splitSentence(std::string query, RecursiveMap &results);
    string_vector splitSentence2(std::string query);

  private:
    struct Node;
    using ChildrenType = std::unordered_map<char, std::unique_ptr<Node>>;
    struct Node {
        ChildrenType children;
        bool isEndOfWord = false;
        auto hasChild(char ch) -> bool;
    };

    Node wRoot;
    Node sRoot;

    // auto remove_(std::string key, int depth) -> bool;
    auto insert_(Node &root, std::string key) -> void;
    auto find_(Node &root, std::string query) -> Node *;
    auto dfs_(Node *root, std::string query, std::string prefix,
              std::vector<std::string> &results, size_t maxDepth = 0) -> void;
};

} // namespace khiin::engine

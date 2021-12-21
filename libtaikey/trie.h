#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace TaiKey {

using VStr = std::vector<std::string>;

struct RecursiveMap {
    std::unordered_map<std::string, RecursiveMap> map;
};

class Trie {
    struct Node;

  public:
    Trie();
    Trie(VStr wordlist);
    auto insert(std::string key) -> void;
    auto remove(std::string key) -> bool;
    auto containsWord(std::string query) -> bool;
    auto containsPrefix(std::string query) -> bool;
    auto autocomplete(std::string query, size_t maxDepth = 0) -> VStr;
    auto autocompleteTone(std::string query) -> VStr;

    // Can delete
    void Trie::splitSentence(std::string query, RecursiveMap &results);
    VStr Trie::splitSentence2(std::string query);

  private:
    using ChildrenType = std::unordered_map<char, std::unique_ptr<Node>>;
    struct Node {
        ChildrenType children;
        bool isEndOfWord = false;
        auto hasChild(char ch) -> bool;
    };

    Node root;

    // auto remove_(std::string key, int depth) -> bool;
    auto find_(std::string query) -> Node *;
    auto dfs_(Node *root, std::string query, std::string prefix,
              std::vector<std::string> &results, size_t maxDepth = 0) -> void;
};

} // namespace TaiKey

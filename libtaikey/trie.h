#pragma once

#include <string>
#include <unordered_map>

namespace TaiKey {

class TNode {
  public:
    void insert(std::string key);
    bool remove(std::string key);
    bool searchExact(std::string query);
    bool isPrefix(std::string query);
    std::vector<std::string> autocomplete(std::string query);
    std::vector<std::string> autocompleteTone(std::string query);

  private:
    std::unordered_map<char, TNode *> children_;
    bool isEndOfWord_ = false;

    bool remove_(std::string key, int depth);
    bool hasChildren_();
    bool hasChild_(char ch);
    TNode *findNode_(std::string query);
    void dfs_(TNode *root, std::string query, std::string prefix, std::vector<std::string> &results);
};

} // namespace TaiKey

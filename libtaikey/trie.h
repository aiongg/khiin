#pragma once

#include <string>
#include <unordered_map>

namespace TaiKey {

using word_wt = std::pair<std::string, int>;
using word_wt_v = std::vector<word_wt>;

struct TNode {
    std::unordered_map<char, TNode *> children;
    word_wt_v leaves;

    void insert(std::string key, word_wt value);
    bool remove(std::string key, std::string value);
    bool hasChildren();
    bool leafCount();
    word_wt_v searchExact(std::string query);

  private:
    bool remove_(std::string key, std::string value, int depth);
    bool removeValue_(std::string value);
};

} // namespace TaiKey

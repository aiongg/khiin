#include "trie.h"

namespace TaiKey {

void TNode::insert(std::string key, word_wt value) {
    TNode *curr = this;

    for (auto &ch : key) {
        if (curr->children.find(ch) == curr->children.end()) {
            curr->children[ch] = new TNode; // need parens?
        }

        curr = curr->children[ch];
    }

    curr->leaves.push_back(value);
}

bool TNode::remove(std::string key, std::string value) {
    return remove_(key, value, 0);
}

bool TNode::remove_(std::string key, std::string value, int depth) {
    // Last node
    if (depth == key.size()) {
        this->removeValue_(value);

        // No more children or leaves -- delete the node
        if (!this->hasChildren() && this->leafCount() == 0) {
            delete this;
            return true;
        }

        return false;
    }

    // Not yet last node, recurse on key at depth + 1
    char index = key[depth];
    bool childRemoved = this->children[index]->remove_(key, value, depth + 1);

    if (childRemoved) {
        this->children.erase(index);

        if (!this->hasChildren() && this->leafCount() == 0 && depth > 0) {
            delete this;
            return true;
        }
    }

    return false;
}

bool TNode::hasChildren() {
    for (const auto &it : this->children) {
        if (it.second != NULL) {
            return true;
        }
    }

    return false;
}

bool TNode::leafCount() { return this->leaves.size(); }

word_wt_v TNode::search(std::string query) {
    TNode *curr = this;
    for (char ch : query) {
        if (curr->children.find(ch) == curr->children.end()) {
            return word_wt_v();
        }

        curr = curr->children[ch];
    }

    return curr->leaves;
}

bool TNode::removeValue_(std::string value) {
    word_wt_v::const_iterator it;

    for (it = leaves.begin(); it != leaves.end(); it++) {
        if ((*it).first == value) {
            leaves.erase(it);
            return true;
        }
    }

    return false;
}

} // namespace TaiKey

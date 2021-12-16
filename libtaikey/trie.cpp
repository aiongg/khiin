#include <iterator>
#include <unordered_map>

#include "trie.h"

namespace TaiKey {

// Public

void TNode::insert(std::string key) {
    TNode *curr = this;

    for (auto &ch : key) {
        if (curr->children_.find(ch) == curr->children_.end()) {
            curr->children_[ch] = new TNode; // need parens?
        }

        curr = curr->children_[ch];
    }

    curr->isEndOfWord_ = true;
}

bool TNode::remove(std::string key) { return remove_(key, 0); }

bool TNode::searchExact(std::string query) {
    TNode *found = findNode_(query);
    return found && found->isEndOfWord_;
}

bool TNode::isPrefix(std::string query) {
    TNode *found = findNode_(query);
    return found && (found->isEndOfWord_ || found->hasChildren_());
}

std::vector<std::string> TNode::autocomplete(std::string query,
                                             size_t maxDepth) {
    TNode *found = findNode_(query);
    std::vector<std::string> ret;

    if (!found) {
        return ret;
    }

    if (found->isEndOfWord_ && !found->hasChildren_()) {
        ret.push_back(query);
        return ret;
    }

    dfs_(found, query, "", ret, maxDepth);

    return ret;
}

std::vector<std::string> TNode::autocompleteTone(std::string query) {
    std::vector<std::string> ret;

    if (isdigit(query.back())) {
        return ret;
    }

    TNode *found = findNode_(query);

    if (!found) {
        return ret;
    }

    for (auto ch : "123456789") {
        if (found->hasChild_(ch) && found->children_[ch]->isEndOfWord_) {
            ret.push_back(query + ch);
        }
    }

    return ret;
}

void TNode::splitSentence(std::string query, RecursiveMap &results) {
    std::string syl;

    std::string::iterator it;
    for (it = query.begin(); it < query.end(); it++) {
        syl += *it;
        auto prefixes = autocomplete(syl, syl.size());

        if (prefixes.size() > 0) {
            for (auto &it2 : prefixes) {
                if (syl == it2) {
                    results.map[syl] = RecursiveMap();
                    splitSentence(std::string(it + 1, query.end()), results.map[syl]);
                }
            }
        }
    }
}

// Private

bool TNode::remove_(std::string key, int depth) {
    // Last node
    if (depth == key.size()) {
        this->isEndOfWord_ = false;

        // No other children -- delete the node
        if (!this->hasChildren_()) {
            delete this;
            return true;
        }

        return false;
    }

    // Not yet last node, recurse on key at depth + 1
    char index = key[depth];
    bool childRemoved = this->children_[index]->remove_(key, depth + 1);

    if (childRemoved) {
        this->children_.erase(index);

        if (!this->hasChildren_() && depth > 0) {
            delete this;
            return true;
        }
    }

    return false;
}

bool TNode::hasChildren_() {
    for (const auto &it : this->children_) {
        if (it.second != NULL) {
            return true;
        }
    }

    return false;
}

bool TNode::hasChild_(char ch) {
    for (auto &it : children_) {
        if (it.first == ch) {
            return true;
        }
    }
    return false;
}

TNode *TNode::findNode_(std::string query) {
    TNode *curr = this;

    std::string::iterator it;
    for (it = query.begin(); it < query.end(); it++) {
        if (curr->children_.find(*it) == curr->children_.end()) {
            return nullptr;
        }

        curr = curr->children_[*it];
    }

    return curr;
}

void TNode::dfs_(TNode *root, std::string prefix, std::string suffix,
                 std::vector<std::string> &results, size_t maxDepth) {
    if (root->isEndOfWord_) {
        results.push_back(prefix + suffix);
    }

    if (!hasChildren_() || --maxDepth == 0) {
        return;
    }

    for (const auto &it : root->children_) {
        dfs_(it.second, prefix, suffix + it.first, results, maxDepth);
    }
}

} // namespace TaiKey

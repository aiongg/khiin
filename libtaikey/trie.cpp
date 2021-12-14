#include <iterator>

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

word_wt_v TNode::searchExact(std::string query) {
    TNode *curr = this;

    std::string::iterator it;
    for (it = query.begin(); it < query.end(); it++) {
        if (curr->children.find(*it) == curr->children.end()) {
            return word_wt_v();
        }

        curr = curr->children[*it];
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

//void crawlSentences(TNode* root, std::string input,
//    std::vector<std::string> results) {
//    std::string word = "";
//    std::string::iterator it;
//
//    for (it = input.begin(); it < input.end(); it++) {
//        word += *it;
//        word_wt_v found = root->search(word);
//        if (found.size() > 0) {
//            word_wt_v::iterator jt;
//            for (jt = found.begin(); jt < found.end(); jt++) {
//                results.push_back((*jt).first);
//            }
//        }
//    }
//}

TNode *tkTrie;

} // namespace TaiKey

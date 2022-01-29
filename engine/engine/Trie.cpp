#include <iterator>
#include <unordered_map>

#include "common.h"
#include "trie.h"

namespace khiin::engine {

using namespace std::literals::string_literals;

// Public

Trie::Trie() {}

Trie::Trie(const string_vector &keys) {
    for (auto &key : keys) {
        this->insert_(root, key);
    }
}

void Trie::Insert(std::string_view key) {
    return insert_(root, key);
}

bool Trie::Remove(std::string_view key) {
    auto onlyChildNodes = std::vector<std::tuple<char, Node *, bool>>();
    auto curr = &root;

    for (auto it = key.begin(); it != key.end(); it++) {
        auto found = curr->children.find(*it);

        if (found == curr->children.end()) {
            return false; // Key not in Trie
        }

        curr = curr->children[*it].get();

        if (std::next(it) == key.end() && curr->isEndOfWord) {
            curr->isEndOfWord = false;
        }

        if (curr->children.size() > 1) {
            onlyChildNodes.push_back(std::make_tuple(*it, curr, false));
        } else {
            onlyChildNodes.push_back(std::make_tuple(*it, curr, true));
        }
    }

    for (auto it = onlyChildNodes.rbegin(); it != onlyChildNodes.rend(); it++) {
        if (it == onlyChildNodes.rbegin()) {
            continue;
        }

        auto &onlyChild = std::get<2>(*it);
        auto &prevOnlyChild = std::get<2>(*std::prev(it));

        if (!onlyChild && prevOnlyChild ||
            std::next(it) == onlyChildNodes.rend()) {
            auto &chr = std::get<0>(*it);
            auto &node = std::get<1>(*it);

            // does smart pointer need reset?
            // node->children[chr].reset();
            node->children.erase(chr);
            return true;
        }
    }

    return false;
}

auto Trie::ContainsWord(std::string_view query) -> bool {
    auto found = find_(root, query);
    return found != nullptr && found->isEndOfWord;
}

auto Trie::ContainsPrefix(std::string_view query) -> bool {
    auto found = find_(root, query);
    return found != nullptr &&
           (found->isEndOfWord || found->children.size() > 0);
}

auto Trie::Autocomplete(std::string const &query, size_t maxDepth) -> string_vector {
    auto ret = string_vector();
    auto found = find_(root, query);

    if (found == nullptr) {
        return ret;
    }

    if (found->isEndOfWord && found->children.size() == 0) {
        ret.push_back(query);
        return ret;
    }

    dfs_(found, query, "", ret, maxDepth);

    return ret;
}

//auto Trie::autocompleteTone(std::string query) -> string_vector {
//    auto ret = string_vector();
//
//    if (isdigit(query.back())) {
//        return ret;
//    }
//
//    Node *found = find_(root, query);
//
//    if (!found) {
//        return ret;
//    }
//
//    static auto tones = "1234567890"s;
//
//    for (auto t : tones) {
//        if (found->hasChild(t) && found->children[t]->isEndOfWord) {
//            ret.push_back(query + t);
//        }
//    }
//
//    return ret;
//}

auto Trie::FindKeys(std::string_view query, bool fuzzy, string_vector &results)
    -> void {
    results.clear();

    if (query.empty()) {
        return;
    }

    if (!root.hasChild(query[0])) {
        return;
    }

    auto curr = &root;
    static auto tones = "1234567890"s;

    for (auto it = query.begin(); it != query.end(); it++) {
        if (curr->children.find(*it) == curr->children.end()) {
            return;
        }

        auto substr = std::string(query.begin(), std::next(it));
        curr = curr->children[*it].get();

        if (curr->isEndOfWord) {
            results.push_back(substr);
        }

        if (fuzzy) {
            if (it + 1 != query.end() && isdigit(*(it + 1))) {
                // tone provided, skip this one
                continue;
            }

            if (it + 1 != query.end() && isdigit(*it) && *(it + 1) != '0' &&
                // only check for khin after provided tone
                curr->hasChild('0') && curr->children['0']->isEndOfWord) {
                results.push_back(substr + '0');
                continue;
            }

            // otherwise check all tones
            for (auto jt : tones) {
                if (curr->hasChild(jt) && curr->children[jt]->isEndOfWord) {
                    results.push_back(substr + jt);

                    // final khin after tone number
                    if (curr->children[jt]->hasChild('0') &&
                        curr->children[jt]->children['0']->isEndOfWord) {
                        results.push_back(substr + jt + '0');
                    }
                }
            }
        }
    }
}

// Private

// Node

auto Trie::Node::hasChild(char ch) -> bool {
    return (children.find(ch) != children.end());
}

// Trie

auto Trie::insert_(Node &root, std::string_view key) -> void {
    auto curr = &root;

    for (auto &ch : key) {
        if (curr->children.find(ch) == curr->children.end()) {
            curr->children[ch] = std::make_unique<Node>();
        }

        curr = curr->children[ch].get();
    }

    curr->isEndOfWord = true;
}

auto Trie::find_(Node &root, std::string_view query) -> Node * {
    auto curr = &root;

    for (auto it = query.begin(); it < query.end(); it++) {
        if (curr->children.find(*it) == curr->children.end()) {
            return nullptr;
        }

        curr = curr->children[*it].get();
    }

    return curr;
}

auto Trie::dfs_(Node *node, std::string const &prefix, std::string const &suffix,
                std::vector<std::string> &results, size_t maxDepth) -> void {
    if (node->isEndOfWord) {
        results.push_back(prefix + suffix);
    }

    if (node->children.size() == 0 || --maxDepth == 0) {
        return;
    }

    for (const auto &it : node->children) {
        dfs_(it.second.get(), prefix, suffix + it.first, results, maxDepth);
    }
}

} // namespace khiin::engine

#include <iterator>
#include <unordered_map>

#include "trie.h"

namespace TaiKey {

using namespace std::literals::string_literals;

// Public

Trie::Trie() {}

Trie::Trie(std::vector<std::string> wordlist) {
    for (auto &word : wordlist) {
        this->insert(word);
    }
}

auto Trie::insert(std::string key) -> void {
    auto curr = &root;

    for (auto &ch : key) {
        if (curr->children.find(ch) == curr->children.end()) {
            curr->children[ch] = std::make_unique<Node>();
        }

        curr = curr->children[ch].get();
    }

    curr->isEndOfWord = true;
}

bool Trie::remove(std::string key) {
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

auto Trie::containsWord(std::string query) -> bool {
    auto found = find_(query);
    return found != nullptr && found->isEndOfWord;
}

auto Trie::containsPrefix(std::string query) -> bool {
    auto found = find_(query);
    return found != nullptr &&
           (found->isEndOfWord || found->children.size() > 0);
}

auto Trie::autocomplete(std::string query, size_t maxDepth) -> VStr {
    auto ret = VStr();
    auto found = find_(query);

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

auto Trie::autocompleteTone(std::string query) -> VStr {
    auto ret = VStr();

    if (isdigit(query.back())) {
        return ret;
    }

    Node *found = find_(query);

    if (!found) {
        return ret;
    }

    static auto tones = "1234567890"s;

    for (auto t : tones) {
        if (found->hasChild(t) && found->children[t]->isEndOfWord) {
            ret.push_back(query + t);
        }
    }

    return ret;
}

auto Trie::getAllWords(std::string query, bool isToneless, VStr &results)
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

        if (isToneless) {
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

// Can delete
void Trie::splitSentence(std::string query, RecursiveMap &results) {
    std::string syl;

    std::string::iterator it;
    for (it = query.begin(); it < query.end(); it++) {
        syl += *it;
        auto prefixes = autocomplete(syl, syl.size());

        if (prefixes.size() > 0) {
            for (auto &it2 : prefixes) {
                if (syl == it2) {
                    results.map[syl] = RecursiveMap();
                    splitSentence(std::string(it + 1, query.end()),
                                  results.map[syl]);
                }
            }
        }
    }
}

// Can delete
std::vector<std::string> Trie::splitSentence2(std::string query) {
    std::vector<std::string> results;
    std::string syl;
    std::string::iterator it;

    if (query.empty()) {
        return results;
    }

    for (it = query.begin(); it != query.end(); it++) {
        syl += *it;
        if (containsWord(syl)) {
            if (syl == query) {
                results.push_back(syl);
            } else {
                auto remainderResult =
                    splitSentence2(std::string(it + 1, query.end()));
                for (auto &res : remainderResult) {
                    results.push_back(syl + " " + res);
                }
            }
        }
    }

    if (results.empty()) {
        results.push_back(query);
    }

    return results;
}

// Private

// Node

auto Trie::Node::hasChild(char ch) -> bool {
    return (children.find(ch) != children.end());
}

// Trie

auto Trie::find_(std::string query) -> Node * {
    auto curr = &root;

    for (auto it = query.begin(); it < query.end(); it++) {
        if (curr->children.find(*it) == curr->children.end()) {
            return nullptr;
        }

        curr = curr->children[*it].get();
    }

    return curr;
}

auto Trie::dfs_(Node *node, std::string prefix, std::string suffix,
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

} // namespace TaiKey

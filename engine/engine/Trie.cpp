#include <iterator>
#include <unordered_map>

#include "common.h"
#include "trie.h"

namespace khiin::engine {

namespace {

struct Node {
    using ChildrenType = std::unordered_map<char, std::unique_ptr<Node>>;
    ChildrenType children;
    bool end_of_word = false;

    inline bool HasChild(char ch) {
        return children.find(ch) != children.end();
    }
};

class TrieImpl : public Trie {
  public:
    TrieImpl() = default;
    TrieImpl(std::vector<std::string> const &words) {
        for (auto const &word : words) {
            Insert(word);
        }
    }

    virtual void Insert(std::string_view key) override {
        auto curr = &root;

        for (auto &ch : key) {
            if (curr->children.find(ch) == curr->children.end()) {
                curr->children[ch] = std::make_unique<Node>();
            }

            curr = curr->children[ch].get();
        }

        curr->end_of_word = true;
    }

    virtual bool Remove(std::string_view key) override {
        auto onlyChildNodes = std::vector<std::tuple<char, Node *, bool>>();
        auto curr = &root;

        for (auto it = key.begin(); it != key.end(); it++) {
            auto found = curr->children.find(*it);

            if (found == curr->children.end()) {
                return false; // Key not in Trie
            }

            curr = curr->children[*it].get();

            if (std::next(it) == key.end() && curr->end_of_word) {
                curr->end_of_word = false;
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

            if (!onlyChild && prevOnlyChild || std::next(it) == onlyChildNodes.rend()) {
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

    virtual bool ContainsWord(std::string_view query) override {
        auto found = Find(query);
        return found != nullptr && found->end_of_word;
    }

    virtual bool ContainsPrefix(std::string_view query) override {
        auto found = Find(query);
        return found != nullptr && (found->end_of_word || found->children.size() > 0);
    }

    virtual string_vector Autocomplete(std::string const &query, size_t maxDepth) override {
        auto ret = string_vector();
        auto found = Find(query);

        if (found == nullptr) {
            return ret;
        }

        if (found->end_of_word && found->children.size() == 0) {
            ret.push_back(query);
            return ret;
        }

        Dfs(found, query, "", ret, maxDepth);

        return ret;
    }

    virtual void FindWords(std::string_view query, string_vector &results) {
        results.clear();

        if (query.empty()) {
            return;
        }

        auto curr = &root;
        for (auto it = query.begin(); it != query.end(); it++) {
            if (curr->children.find(*it) == curr->children.end()) {
                return;
            }

            curr = curr->children[*it].get();
            ++it;
            if (curr->end_of_word) {
                results.push_back(std::string(query.begin(), it));
            }
        }
    }

    virtual void FindKeys(std::string_view query, bool fuzzy, string_vector &results) {
        results.clear();

        if (query.empty()) {
            return;
        }

        if (!root.HasChild(query[0])) {
            return;
        }

        auto curr = &root;
        static auto tones = std::string("1234567890");

        for (auto it = query.begin(); it != query.end(); it++) {
            if (curr->children.find(*it) == curr->children.end()) {
                return;
            }

            auto substr = std::string(query.begin(), std::next(it));
            curr = curr->children[*it].get();

            if (curr->end_of_word) {
                results.push_back(substr);
            }

            if (fuzzy) {
                if (it + 1 != query.end() && isdigit(*(it + 1))) {
                    // tone provided, skip this one
                    continue;
                }

                if (it + 1 != query.end() && isdigit(*it) && *(it + 1) != '0' &&
                    // only check for khin after provided tone
                    curr->HasChild('0') && curr->children['0']->end_of_word) {
                    results.push_back(substr + '0');
                    continue;
                }

                // otherwise check all tones
                for (auto jt : tones) {
                    if (curr->HasChild(jt) && curr->children[jt]->end_of_word) {
                        results.push_back(substr + jt);

                        // final khin after tone number
                        if (curr->children[jt]->HasChild('0') && curr->children[jt]->children['0']->end_of_word) {
                            results.push_back(substr + jt + '0');
                        }
                    }
                }
            }
        }
    }

  private:
    auto Find(std::string_view query) -> Node * {
        auto curr = &root;

        for (auto it = query.begin(); it < query.end(); it++) {
            if (curr->children.find(*it) == curr->children.end()) {
                return nullptr;
            }

            curr = curr->children[*it].get();
        }

        return curr;
    }

    auto Dfs(Node *node, std::string const &prefix, std::string const &suffix, std::vector<std::string> &results,
             size_t maxDepth) -> void {
        if (node->end_of_word) {
            results.push_back(prefix + suffix);
        }

        if (node->children.size() == 0 || --maxDepth == 0) {
            return;
        }

        for (const auto &it : node->children) {
            Dfs(it.second.get(), prefix, suffix + it.first, results, maxDepth);
        }
    }

    Node root;
};

} // namespace

// Public

Trie *Trie::Create() {
    return new TrieImpl();
}

static Trie *Create(std::vector<std::string> const &words) {
    return new TrieImpl(words);
}

// auto Trie::autocompleteTone(std::string query) -> string_vector {
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

// Private

// Node

// Trie

} // namespace khiin::engine

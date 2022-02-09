#include <iterator>
#include <queue>
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

    virtual bool HasKey(std::string_view query) override {
        auto found = Find(query);
        return found != nullptr && found->end_of_word;
    }

    virtual bool StartsWithKey(std::string_view query) override {
        if (query.empty()) {
            return false;
        }

        auto curr = &root;
        for (auto it = query.begin(); it != query.end(); ++it) {
            if (curr->end_of_word) {
                return true;
            } else if (curr->children.find(*it) == curr->children.end()) {
                return false;
            }

            curr = curr->children[*it].get();
        }
        return curr->end_of_word;
    }

    virtual bool HasKeyOrPrefix(std::string_view query) override {
        auto found = Find(query);
        return found != nullptr && (found->end_of_word || found->children.size() > 0);
    }

    virtual size_t LongestKeyOf(std::string_view query) override {
        size_t ret = 0;

        if (query.empty()) {
            return ret;
        }

        auto curr = &root;
        for (auto it = query.begin(); it != query.end(); ++it) {
            if (curr->end_of_word) {
                ret = std::distance(query.begin(), it);
            }

            if (curr->children.find(*it) == curr->children.end()) {
                return ret;
            }

            curr = curr->children[*it].get();
        }

        return ret;
    }

    virtual string_vector Autocomplete(std::string const &query, size_t limit, size_t maxDepth) override {
        auto ret = string_vector();
        auto found = Find(query);

        if (found == nullptr) {
            return ret;
        }

        if (found->end_of_word && found->children.size() == 0 && limit == 1) {
            ret.push_back(query);
            return ret;
        }

        BreadthFirstSearch(found, query, ret, limit);
        //DepthFirstSearch(found, query, "", ret, limit, maxDepth);

        return ret;
    }

    virtual void FindKeys(std::string_view query, string_vector &results) {
        results.clear();

        if (query.empty()) {
            return;
        }

        auto curr = &root;
        auto it = query.begin();
        while (it != query.end()) {
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

    void BreadthFirstSearch(Node *start, std::string const &prefix, std::vector<std::string> &result, int limit) {
        auto queue = std::queue<std::pair<std::string, Node *>>();

        if (start->end_of_word) {
            result.push_back(prefix);
        }

        for (auto &c : start->children) {
            queue.push(std::make_pair(std::string(1, c.first), c.second.get()));
        }

        while (!queue.empty()) {
            auto [suffix, node] = std::move(queue.front());
            queue.pop();

            if (node->end_of_word) {
                result.push_back(prefix + suffix);

                if (limit && result.size() >= limit) {
                    return;
                }
            }

            for (auto &c : node->children) {
                queue.push(std::make_pair(suffix + c.first, c.second.get()));
            }

        }
    }

    auto DepthFirstSearch(Node *node, std::string const &prefix, std::string const &suffix,
                          std::vector<std::string> &results, size_t limit, size_t max_depth) -> void {
        if (node->end_of_word) {
            results.push_back(prefix + suffix);
        }

        if (limit != 0 && results.size() >= limit) {
            return;
        }

        if (node->children.size() == 0 || --max_depth == 0) {
            return;
        }

        for (const auto &it : node->children) {
            DepthFirstSearch(it.second.get(), prefix, suffix + it.first, results, limit, max_depth);
        }
    }

    Node root;
};

} // namespace

Trie *Trie::Create() {
    return new TrieImpl();
}

static Trie *Create(std::vector<std::string> const &words) {
    return new TrieImpl(words);
}

} // namespace khiin::engine

#include "Trie.h"

#include <array>
#include <assert.h>
#include <bitset>
#include <deque>
#include <iterator>
#include <queue>
#include <unordered_map>

#include "Splitter.h"
#include "common.h"

namespace khiin::engine {

namespace {

inline int bitscan_forward(uint64_t x) {
    auto bits = std::bitset<64>((x & -x) - 1);
    return bits.count();
}

const int bitscan_reverse(uint64_t x) {
    int ret = 0;
    while (x >>= 1) {
        ++ret;
    }
    return ret;
}

std::vector<int> get_bit_positions(uint64_t bb_ull) {
    auto ret = std::vector<int>();
    while (bb_ull != 0) {
        auto pos = bitscan_forward(bb_ull);
        auto bb = std::bitset<64>(bb_ull);
        bb.set(pos, false);
        bb_ull = bb.to_ullong();
        ret.push_back(pos);
    }
    return ret;
}

struct SplitCost {
    uint64_t split = 0;
    float cost = 0.0f;
};

inline bool IsCheaper(SplitCost &a, SplitCost &b) {
    return a.cost < b.cost;
}

void PushBackIfCheaper(std::vector<SplitCost> &vec, int limit, uint64_t split, float cost) {
    if (vec.size() < limit) {
        vec.push_back(SplitCost{split, cost});
        std::sort(vec.begin(), vec.end(), IsCheaper);
    } else {
        if (cost < vec.back().cost) {
            vec.back().split = split;
            vec.back().cost = cost;
            std::sort(vec.begin(), vec.end(), IsCheaper);
        }
    }
}

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
        // DepthFirstSearch(found, query, "", ret, limit, maxDepth);

        return ret;
    }

    virtual void FindKeys(std::string_view query, std::vector<std::string> &results) {
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

    virtual std::vector<std::vector<int>> AllSplits(std::string_view query, SplitterCostMap const &cost_map) override {
        int limit = 100;
        query = query.substr(0, 64);
        auto query_size = query.size();

        auto table = std::vector<std::vector<SplitCost>>(query.size() + 1, std::vector<SplitCost>());
        table[0].push_back(SplitCost());
        auto qbegin = query.begin();
        auto qend = query.end();

        for (auto start = qbegin; start != qend; ++start) {
            auto start_idx = std::distance(qbegin, start);

            if (table[start_idx + 1].size() == limit) {
                continue;
            }

            auto node = &root;

            for (auto it = start; it != qend; ++it) {
                auto end_idx = std::distance(qbegin, it) + 1;

                if (node->children.find(*it) == node->children.end()) {
                    break;
                }

                node = node->children[*it].get();

                if (node->end_of_word) {
                    auto found = cost_map.find(std::string(start, it + 1));
                    if (found != cost_map.end()) {
                        for (auto &result : table[start_idx]) {
                            auto bits = std::bitset<64>(result.split);
                            bits.set(end_idx, true);
                            auto split = bits.to_ullong();
                            auto cost = result.cost + found->second;
                            PushBackIfCheaper(table[end_idx], 10, split, cost);
                        }
                    }
                }
            }
        }

        auto ret = std::vector<std::vector<int>>();

        // std::sort(result.begin(), result.end(), IsCheaper);
        // for (auto &cost : result) {
        //    ret.push_back(get_bit_positions(cost.split));
        //}

        return ret;
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

    void DepthFirstSearch(Node *node, std::string const &prefix, std::string const &suffix,
                          std::vector<std::string> &results, size_t limit, size_t max_depth) {
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

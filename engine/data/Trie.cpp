#include "Trie.h"

#include <array>
#include <assert.h>
#include <bitset>
#include <deque>
#include <iterator>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "utils/common.h"
#include "utils/utils.h"

#include "Splitter.h"

namespace khiin::engine {

namespace {

struct SplitCost {
    bool operator<(SplitCost const &rhs) const {
        return cost < rhs.cost;
    }
    uint64_t split = 0;
    float cost = 0.0F;
};

/**
 * |vec| is sorted by cost from low to high.
 * If size of |vec| is under the limit, push back directly.
 * Otherwise, check cost against the last element (highest cost), and
 * replace/re-sort only if it is cheaper.
 */
void SaveIfCheaper(std::vector<SplitCost> &vec, int limit, uint64_t split, float cost) {
    if (vec.size() < limit) {
        vec.push_back(SplitCost{split, cost});
        std::sort(vec.begin(), vec.end());
    } else {
        if (cost < vec.back().cost) {
            vec.back().split = split;
            vec.back().cost = cost;
            std::sort(vec.begin(), vec.end());
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

    void Insert(std::string_view key) override {
        auto *curr = &root;

        for (auto ch : key) {
            if (curr->children.find(ch) == curr->children.end()) {
                curr->children[ch] = std::make_unique<Node>();
            }

            curr = curr->children[ch].get();
        }

        curr->end_of_word = true;
    }

    bool Remove(std::string_view key) override {
        auto onlyChildNodes = std::vector<std::tuple<char, Node *, bool>>();
        auto *curr = &root;

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

            if ((!onlyChild && prevOnlyChild) || std::next(it) == onlyChildNodes.rend()) {
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

    bool HasKey(std::string_view query) override {
        auto found = Find(query);
        return found != nullptr && found->end_of_word;
    }

    bool StartsWithKey(std::string_view query) override {
        if (query.empty()) {
            return false;
        }

        auto *curr = &root;
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

    bool HasKeyOrPrefix(std::string_view query) override {
        auto *found = Find(query);
        return found != nullptr && (found->end_of_word || !found->children.empty());
    }

    size_t LongestKeyOf(std::string_view query) override {
        size_t ret = 0;

        if (query.empty()) {
            return ret;
        }

        auto *curr = &root;
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

    std::vector<std::string> Autocomplete(std::string const &query, int limit, int maxDepth) override {
        auto ret = std::vector<std::string>();
        auto *found = Find(query);

        if (found == nullptr) {
            return ret;
        }

        if (found->end_of_word && found->children.empty() && limit == 1) {
            ret.push_back(query);
            return ret;
        }

        BreadthFirstSearch(found, query, ret, limit);
        // DepthFirstSearch(found, query, "", ret, limit, maxDepth);

        return ret;
    }

    void FindKeys(std::string_view query, std::vector<std::string> &results) {
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

    std::vector<std::vector<int>> Multisplit(std::string_view query, WordCostMap const &cost_map,
                                                     uint32_t limit) override {
        query = query.substr(0, 63);
        //auto query_size = query.size();

        /**
         * The table contains one row for each index in the query string, up to max of 64
         * Each row contains a result vector, which will reach a max size of |limit|
         * Each result contains a uint64_t called |split| representing a bitfield,
         * where the position of each set bit represents a split in the query string,
         * plus the total cost associated with that split pattern.
         */
        auto table = std::vector<std::vector<SplitCost>>(query.size() + 1, std::vector<SplitCost>());
        table[0].push_back(SplitCost());
        auto qbegin = query.begin();
        auto qend = query.end();

        /**
         * |start| is a pointer to the letter one past the end of the current split index
         * e.g., each result in the table at index |start_idx| has a split at that position
         */
        for (auto start = qbegin; start != qend; ++start) {
            auto start_idx = std::distance(qbegin, start);
            auto *node = &root;

            if (table[start_idx].empty()) {
                continue;
            }

            /**
             * Iterate through the remainder of the query string with pointer |it|.
             * When query[start, it] is a word, check if the cost is cheaper than
             * the most expensive result currently stored in table[it_idx]. If so,
             * replace the most expensive one with the cheaper one.
             */
            for (auto it = start; it != qend; ++it) {
                auto it_idx = std::distance(qbegin, it) + 1;

                if (node->children.find(*it) == node->children.end()) {
                    break;
                }

                node = node->children[*it].get();

                if (node->end_of_word) {
                    if (auto found = cost_map.find(std::string(start, it + 1)); found != cost_map.end()) {
                        for (auto &result : table[start_idx]) {
                            auto bits = std::bitset<64>(result.split);
                            bits.set(it_idx, true);
                            auto split = bits.to_ullong();
                            auto cost = result.cost + found->second;
                            SaveIfCheaper(table[it_idx], limit, split, cost);
                        }
                    }
                }
            }
        }

        auto ret = std::vector<std::vector<int>>();
        auto last_row = table.end() - 1;
        while (last_row != table.begin() && last_row->empty()) {
            --last_row;
        }

        for (auto &result : *last_row) {
            ret.push_back(utils::bitpositions(result.split));

            if (ret.size() >= limit) {
                last_row = table.begin();
            }
        }

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

        if (node->children.empty() || --max_depth == 0) {
            return;
        }

        for (const auto &it : node->children) {
            DepthFirstSearch(it.second.get(), prefix, suffix + it.first, results, limit, max_depth);
        }
    }

    Node root;
};

} // namespace

std::unique_ptr<Trie> Trie::Create() {
    return std::make_unique<TrieImpl>();
}

std::unique_ptr<Trie> Create(std::vector<std::string> const &words) {
    return std::make_unique<TrieImpl>(words);
}

} // namespace khiin::engine

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

#include "splitter.h"

using namespace std::string_literals;

namespace khiin::engine {
namespace {
using WordSet = std::unordered_set<std::string>;

inline auto isDigit(std::string const &str) {
    std::stringstream s;
    s << str;
    double d = 0;
    char c;
    return (s >> d) ? !(s >> c) : false;
}

struct SplitCheckResult {
    std::vector<bool> splits_at;
    std::vector<int> split_indices;
};

SplitCheckResult CheckSplittable(WordSet const &words, std::string const &query) {
    auto size = query.size();
    std::vector<bool> splits_at(size + 1, false);
    std::vector<int> split_indices;
    split_indices.push_back(-1);

    for (auto i = 0; i < size; ++i) {
        auto n_splits = static_cast<int>(split_indices.size());

        for (auto j = n_splits - 1; j >= 0; j--) {
            std::string substr =
                query.substr(static_cast<size_t>(split_indices[j] + 1), static_cast<size_t>(i - split_indices[j]));

            if (words.find(substr) != words.end()) {
                splits_at[i] = true;
                split_indices.push_back(i);
                break;
            }
        }
    }

    return SplitCheckResult{std::move(splits_at), std::move(split_indices)};
}

} // namespace

Splitter::Splitter() {}

Splitter::Splitter(std::vector<std::string> const &words_by_frequency) {
    std::copy(words_by_frequency.cbegin(), words_by_frequency.cend(), std::inserter(m_word_set, m_word_set.begin()));

    auto log_size = static_cast<float>(log(words_by_frequency.size()));

    auto idx = 0;
    for (auto &it : words_by_frequency) {
        m_cost_map[it] = static_cast<float>(log((idx + 1) * log_size));
        m_max_word_length = std::max(m_max_word_length, (int)it.size());
        ++idx;
    }
}

size_t Splitter::MaxSplitSize(std::string const &input) const {
    if (input.empty()) {
        return 0;
    }

    auto result = CheckSplittable(m_word_set, input);
    return static_cast<size_t>(result.split_indices.back() + 1);
}

bool Splitter::CanSplit(std::string const &input) const {
    if (input.empty()) {
        return true;
    }

    auto result = CheckSplittable(m_word_set, input);
    return result.splits_at[input.size() - 1];
}

void Splitter::Split(std::string const &input, std::vector<std::string> &result) const {
    result.clear();
    if (input.empty()) {
        return;
    }

    const auto len = input.size();

    std::vector<std::pair<float, int>> cost;
    cost.reserve(len + 1);
    cost.push_back(std::make_pair(0.0f, -1));
    auto chunk = std::string();
    auto curr_cost = 0.0f;

    for (auto i = 1; i < len + 1; i++) {
        auto min_cost = cost[i - 1].first + 9e9f;
        auto min_cost_idx = i - 1;

        for (auto j = i - m_max_word_length > 0 ? i - m_max_word_length : 0; j < i; j++) {

            chunk = input.substr(j, i - j);

            if (m_cost_map.find(chunk) == m_cost_map.end()) {
                continue;
            }

            curr_cost = cost[j].first + m_cost_map.at(chunk);
            if (curr_cost <= min_cost) {
                min_cost = curr_cost;
                min_cost_idx = j;
            }
        }

        cost.push_back(std::make_pair(min_cost, min_cost_idx));
    }

    size_t n = len;
    size_t preIndex;

    while (n > 0) {
        preIndex = cost[n].second;
        auto insertStr = input.substr(preIndex, n - preIndex);

        if (!result.empty() && isDigit(insertStr + result[0])) {
            result[0] = insertStr + result[0];
        } else {
            result.insert(result.begin(), insertStr);
        }
        n = preIndex;
    }
}

SplitterCostMap const & Splitter::cost_map() const {
    return m_cost_map;
}

} // namespace khiin::engine

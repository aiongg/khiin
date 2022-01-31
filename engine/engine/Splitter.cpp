#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

#include "splitter.h"

using namespace std::string_literals;

namespace khiin::engine {

inline auto isDigit(std::string const &str) {
    std::stringstream s;
    s << str;
    double d = 0;
    char c;
    return (s >> d) ? !(s >> c) : false;
}

Splitter::Splitter() {}

Splitter::Splitter(const string_vector &input_id_map) {
    std::copy(input_id_map.cbegin(), input_id_map.cend(), std::inserter(m_word_set, m_word_set.begin()));

    auto logListSize = static_cast<float>(log(input_id_map.size()));

    size_t idx = 0;
    for (auto &it : input_id_map) {
        m_cost_map[it] = static_cast<float>(log((idx + 1) * logListSize));
        m_max_word_length = std::max(m_max_word_length, (int)it.size());
        ++idx;
    }
}

bool Splitter::CanSplit(std::string const &input) {
    auto len = input.size();
    if (len == 0) {
        return true;
    }

    std::vector<bool> dp(len + 1, false);
    std::vector<int> matchedIndex;
    matchedIndex.push_back(-1);

    for (int i = 0; i < len; i++) {
        auto mSize = static_cast<int>(matchedIndex.size());
        auto found = false;

        for (int j = mSize - 1; j >= 0; j--) {
            std::string substr = input.substr(matchedIndex[j] + 1, i - matchedIndex[j]);

            if (m_word_set.find(substr) != m_word_set.end()) {
                found = true;
            }
        }

        if (found) {
            dp[i] = true;
            matchedIndex.push_back(i);
        }
    }

    return dp[len - 1];
}

void Splitter::Split(std::string const &input, string_vector &result) {
    result.clear();
    if (input.empty()) {
        return;
    }

    auto lcInput(input);
    std::transform(lcInput.begin(), lcInput.end(), lcInput.begin(), (int (*)(int))tolower);
    const auto len = lcInput.size();

    std::vector<std::pair<float, int>> cost;
    cost.reserve(len + 1);
    cost.push_back(std::make_pair(0.0f, -1));
    auto chunk = std::string();
    auto curCost = 0.0f;

    for (auto i = 1; i < len + 1; i++) {
        auto minCost = cost[i - 1].first + 9e9f;
        auto minCostIdx = i - 1;

        for (auto j = i - m_max_word_length > 0 ? i - m_max_word_length : 0; j < i; j++) {

            chunk = lcInput.substr(j, i - j);

            if (m_cost_map.find(chunk) == m_cost_map.end()) {
                continue;
            }

            curCost = cost[j].first + m_cost_map.at(chunk);
            if (curCost <= minCost) {
                minCost = curCost;
                minCostIdx = j;
            }
        }

        cost.push_back(std::make_pair(minCost, minCostIdx));
    }

    auto n = len;
    decltype(n) preIndex;

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

} // namespace khiin::engine

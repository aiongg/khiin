#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

#include "splitter.h"

using namespace std::string_literals;

namespace TaiKey {

inline auto isDigit(std::string str) {
    std::stringstream s;
    s << str;
    double d = 0;
    char c;
    return (s >> d) ? !(s >> c) : false;
}

Splitter::Splitter() {}

Splitter::Splitter(const VStr &syllableList) {
    std::copy(syllableList.cbegin(), syllableList.cend(),
              std::inserter(syllableSet_, syllableSet_.begin()));

    auto logListSize = static_cast<float>(log(syllableList.size()));

    size_t idx = 0;
    for (auto &it : syllableList) {
        costMap_[it] = static_cast<float>(log((idx + 1) * logListSize));
        maxWordLength_ = std::max(maxWordLength_, (int)it.size());
        ++idx;
    }
}

auto Splitter::canSplit(std::string input) -> bool {
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
            std::string substr =
                input.substr(matchedIndex[j] + 1, i - matchedIndex[j]);

            while (!substr.empty() && isdigit(substr.back())) {
                substr.pop_back();
            }

            if (syllableSet_.find(substr) != syllableSet_.end()) {
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

auto Splitter::split(std::string input) -> VStr {
    auto res = VStr();
    split(input, res);
    return std::move(res);
}

auto Splitter::split(std::string input, VStr &result)
    -> RetVal {
    result.clear();
    if (input.empty()) {
        return RetVal::Error;
    }

    auto lcInput(input);
    std::transform(lcInput.begin(), lcInput.end(), lcInput.begin(),
                   (int (*)(int))tolower);
    const auto len = lcInput.size();

    std::vector<std::pair<float, int>> cost;
    cost.reserve(len + 1);
    cost.push_back(std::make_pair(0.0f, -1));
    auto chunk(""s);
    auto curCost = 0.0f;

    for (auto i = 1; i < len + 1; i++) {
        auto minCost = cost[i - 1].first + 9e9f;
        auto minCostIdx = i - 1;

        for (auto j = i - maxWordLength_ > 0 ? i - maxWordLength_ : 0; j < i;
             j++) {

            chunk = lcInput.substr(j, i - j);

            // Split after digits
            while (!chunk.empty() && isdigit(chunk.back())) {
                chunk.pop_back();
            }

            if (costMap_.find(chunk) == costMap_.end()) {
                continue;
            }

            curCost = cost[j].first + costMap_.at(chunk);
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

    return RetVal::OK;
}

} // namespace TaiKey

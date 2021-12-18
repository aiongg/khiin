#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>

#include "syl_splitter.h"

namespace TaiKey {

inline bool isDigit(std::string str) {
    std::stringstream s;
    s << str;
    double d = 0;
    char c;
    return (s >> d) ? !(s >> c) : false;
}

Splitter::Splitter() {}

Splitter::Splitter(const std::vector<std::string> &syllableList)
    : maxWordLength_(0) {
    size_t logListSize = log(syllableList.size());

    size_t idx = 0;
    for (auto it : syllableList) {
        costMap_[it] = log((idx + 1) * logListSize);
        maxWordLength_ = std::max(maxWordLength_, (int)it.size());
        ++idx;
    }
}

retval_t Splitter::split(std::string input, std::vector<std::string> &result) {
    result.clear();
    if (input.empty()) {
        return TK_ERROR;
    }

    std::string lcInput = input;
    std::transform(lcInput.begin(), lcInput.end(), lcInput.begin(),
                   (int (*)(int))tolower);
    const size_t len = lcInput.size();

    std::vector<std::pair<float, int>> cost;
    cost.reserve(len + 1);
    cost.push_back(std::make_pair(0, -1));
    std::string chunk = "";
    float curCost = 0.0;

    for (int i = 1; i < len + 1; i++) {
        float minCost = cost[i - 1].first + 9e9;
        int minCostIdx = i - 1;

        for (int j = i - maxWordLength_ > 0 ? i - maxWordLength_ : 0; j < i;
             j++) {

            chunk = lcInput.substr(j, i - j);
            if (costMap_.find(chunk) == costMap_.end()) {
                continue;
            }

            curCost = cost[j].first + costMap_.at(chunk);
            if (curCost < minCost) {
                minCost = curCost;
                minCostIdx = j;
            }
        }

        cost.push_back(std::make_pair(minCost, minCostIdx));
    }

    int n = len;
    int preIndex;
    while (n > 0) {
        preIndex = cost[n].second;
        std::string insertStr = input.substr(preIndex, n - preIndex);

        if (!result.empty() && isDigit(insertStr + result[0])) {
            result[0] = insertStr + result[0];
        } else {
            result.insert(result.begin(), insertStr);
        }
        n = preIndex;
    }

    return TK_OK;
}

} // namespace TaiKey

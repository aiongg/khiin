#include <algorithm>
#include <numeric>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/regex.hpp>

#include "candidates.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

using namespace std::literals::string_literals;

CandidateFinder::CandidateFinder(TKDB &db, Splitter &splitter, Trie &trie)
    : db_(db), splitter_(splitter), trie_(trie) {}

static auto toCandidate(Candidate &cr, int inputSize) -> Candidate {
    return Candidate{
        inputSize,
        cr.input,
        cr.output,
        cr.hint,
    };
}

/**
 * Returns the number of `syllables` matched by `word`, starting
 * from the left; 0 if no syllables match
 */
size_t alignedSyllables(const VStr &syllables, std::string word) {
    if (std::find(syllables.begin(), syllables.end(), word) !=
        syllables.end()) {
        return 1;
    }

    static boost::regex toneRe("\\d+");
    auto tonelessWord = boost::regex_replace(word, toneRe, "");
    auto wordIdx = size_t(0);
    auto sylIdx = size_t(0);

    for (auto &syl : syllables | boost::adaptors::indexed()) {
        if (wordIdx == tonelessWord.size()) {
            return sylIdx;
        }

        if (syl.value().size() > tonelessWord.size() - wordIdx) {
            return 0;
        }

        if (tonelessWord.rfind(syl.value(), wordIdx) == std::string::npos) {
            return 0;
        }

        sylIdx++;
        wordIdx += syl.value().size();
    }

    if (wordIdx == tonelessWord.size()) {
        return sylIdx;
    }

    return -1;
}

// modifies rgrams
auto CandidateFinder::sortCandidatesByBigram_(std::string lgram, int lgramCount,
                                              Candidates &rgrams) {
    auto bigrams = BigramWeights();
    auto rgramOutputs = VStr();
    for (const auto &c : rgrams) {
        rgramOutputs.push_back(c.output);
    }
    db_.selectBigramsFor(lgram, rgramOutputs, bigrams);
    for (auto &c : rgrams) {
        c.bigramWt = static_cast<float>(bigrams.at(c.output)) /
                     static_cast<float>(lgramCount);
    }
    std::sort(rgrams.begin(), rgrams.end(),
              [](Candidate a, Candidate b) -> bool {
                  return a.bigramWt > b.bigramWt;
              });
}

/**
 * Makes database call to look up lgram/rgram pairs, returns
 * index in `&rgrams` of the best match
 */
auto CandidateFinder::findBestCandidateByBigram_(std::string lgram,
                                                 int lgramCount,
                                                 const Candidates &rgrams)
    -> size_t {
    if (lgramCount == 0) {
        return 0;
    }

    auto bigrams = BigramWeights();
    auto rgramOutputs = VStr();
    for (const auto &c : rgrams) {
        rgramOutputs.push_back(c.output);
    }
    db_.selectBigramsFor(lgram, rgramOutputs, bigrams);

    auto bestWeight = 0.0f;
    auto bestMatch = size_t(0);

    for (const auto &c : rgrams | boost::adaptors::indexed(0)) {
        auto weight = static_cast<float>(bigrams[c.value().output]) /
                      static_cast<float>(lgramCount);
        if (weight > bestWeight) {
            bestMatch = c.index();
        }
    }

    return bestMatch;
}

auto CandidateFinder::findBestCandidateBySplitter_(std::string input,
                                                   bool toneless) -> Candidate {
    auto syllables = VStr();
    auto trieWords = VStr();
    auto candRows = Candidates();

    splitter_.split(input, syllables);
    trie_.getAllWords(input, toneless, trieWords);

    if (trieWords.size() == 0) {
        return Candidate{0, syllables[0], syllables[0], syllables[0]};
    }

    auto matched = VStr();
    std::copy_if(trieWords.begin(), trieWords.end(),
                 std::back_inserter(matched), [&](std::string word) {
                     return alignedSyllables(syllables, word) > 0;
                 });

    db_.selectCandidatesFor(matched, candRows);

    if (candRows.empty()) {
        return Candidate{0, syllables[0], syllables[0], syllables[0]};
    }

    return candRows[0];
}

auto CandidateFinder::findCandidates(std::string input, bool toneless,
                                     std::string lgram) -> Candidates {
    auto candidate = Candidate();
    candidate.output = lgram;
    if (!lgram.empty()) {
        candidate.unigramN = db_.getUnigramCount(lgram);
    }
    return findCandidates(input, toneless, candidate);
}

auto CandidateFinder::findCandidates(std::string input, bool toneless,
                                     Candidate lgram) -> Candidates {
    auto rCandidates = Candidates();

    if (input.empty()) {
        return rCandidates;
    }

    auto trieWords = VStr();

    trie_.getAllWords(input, toneless, trieWords);

    if (trieWords.empty()) {
        // TODO
    }

    db_.selectCandidatesFor(trieWords, rCandidates);

    if (lgram.unigramN > 0) {
        sortCandidatesByBigram_(lgram.output, lgram.unigramN, rCandidates);
    }

    for (auto &c : rCandidates) {
        auto i = 0;
        while (i < input.size() && c.ascii[i] == input[i]) {
            i++;
        }
        if (i < c.ascii.size()) {
            c.ascii.erase(c.ascii.begin() + i, c.ascii.end());
        }
    }

    return rCandidates;
}

auto CandidateFinder::findPrimaryCandidate(std::string input, bool toneless,
                                           std::string lgram) -> Candidates {
    auto candidate = Candidate();
    candidate.output = lgram;
    if (!lgram.empty()) {
        candidate.unigramN = db_.getUnigramCount(lgram);
    }
    return findPrimaryCandidate(input, toneless, candidate);
}

/**
 * Candidate::ascii matches input string on output, even if the
 * original result from the database included a tone number and
 * the input string didn't (fuzzy tone mode). That is, the ascii
 * member of a candidate matches exactly the user's raw input and
 * can be directly compared with the raw buffer text.
 */
auto CandidateFinder::findPrimaryCandidate(std::string input, bool toneless,
                                           Candidate cLgram) -> Candidates {
    auto ret = Candidates();

    if (input.empty()) {
        return ret;
    }

    auto lgram = cLgram.output;
    auto lgramCount = cLgram.unigramN;
    auto cbc = Candidate(); // currentBestCandidate
    auto currCandidates = Candidates();

    while (!input.empty()) {
        // case: hyphens at front, add them as separate candidate
        if (input[0] == '-') {
            auto it = input.begin();
            while (it != input.end() && (*it) == '-') {
                it++;
            }

            auto hyphens = std::string(input.begin(), it);
            auto cand = Candidate{0, hyphens, hyphens, hyphens};
            ret.emplace_back(std::move(cand));
            input.erase(input.begin(), it);

            continue;
        }

        auto trieWords = trie_.getAllWords(input, toneless);

        // case: nothing found in the trie, if it forms a prefix
        // when combined with previous candidate, add it; otherwise
        // make a new candidate
        if (trieWords.empty()) {
            auto it = input.begin() + 1;
            auto &prev = ret.size() == 0 ? "" : ret.back().ascii;
            auto canCombine = false;

            auto test = prev + std::string(input.begin(), it);

            while (trie_.containsSyllablePrefix(
                prev + std::string(input.begin(), it))) {
                canCombine = true;
                if (it != input.end()) {
                    it++;
                } else {
                    break;
                }
            }

            auto candstr = std::string(input.begin(), it);
            if (ret.size() > 0 && canCombine) {
                candstr = ret.back().ascii + candstr;
                ret.pop_back();
            }

            auto cand = Candidate{0, candstr, candstr, candstr};
            ret.emplace_back(std::move(cand));
            input.erase(input.begin(), it);
            lgram.clear();
            lgramCount = 0;

            continue;
        }

        // default case: found stuff in the trie
        auto matched = VStr();

        if (lgram == "") {
            // At the beginning, use the syllable splitter to get best result
            auto splitResult = splitter_.split(input);
            std::copy_if(trieWords.begin(), trieWords.end(),
                         std::back_inserter(matched), [&](std::string word) {
                             return word.rfind(splitResult[0], 0) == 0;
                         });
        } else {
            // Otherwise, use the best fit leaving as few dangling characters
            // at the end as possible
            auto stopPos = 0;

            auto canSplit = [&](std::string word) {
                auto i = 0;
                while (i < input.size() && word[i] == input[i]) {
                    i++;
                }

                return splitter_.canSplit(
                    std::string(input.begin() + i, input.end() - stopPos));
            };

            while (matched.size() == 0) {
                std::copy_if(trieWords.begin(), trieWords.end(),
                             std::back_inserter(matched), canSplit);

                if (matched.size() == 0) {
                    stopPos++;
                }
            }
        }

        db_.selectCandidatesFor(matched, currCandidates);

        if (currCandidates.empty()) {
            // Try the splitter again
            lgram.clear();
            lgramCount = 0;
            continue;
        }

        auto idx = 0;

        if (lgram != "") {
            idx = findBestCandidateByBigram_(lgram, lgramCount, currCandidates);
        }

        cbc = currCandidates[idx];

        int i = 0;
        while (i < input.size() && cbc.ascii[i] == input[i]) {
            i++;
        }

        if (i < cbc.ascii.size()) {
            cbc.ascii.erase(cbc.ascii.begin() + i, cbc.ascii.end());
        }

        lgram = cbc.output;
        lgramCount = cbc.unigramN;

        input.erase(0, i);
        ret.push_back(cbc);
    }

    return ret;
}

} // namespace TaiKey

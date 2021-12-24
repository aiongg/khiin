#include <numeric>
#include <regex>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>

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

    static std::regex toneRe("\\d+");
    auto tonelessWord = std::regex_replace(word, toneRe, "");
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
        return Candidate();
    }

    auto matched = VStr();
    std::copy_if(trieWords.begin(), trieWords.end(),
                 std::back_inserter(matched), [&](std::string word) {
                     return alignedSyllables(syllables, word) > 0;
                 });

    db_.selectCandidatesFor(matched, candRows);

    if (candRows.empty()) {
        return Candidate();
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

auto CandidateFinder::findPrimaryCandidate(std::string input, bool toneless,
                                           Candidate cLgram) -> Candidates {
    auto rCandidates = Candidates();

    if (input.empty()) {
        return rCandidates;
    }

    auto lgram = cLgram.output;
    auto lgramCount = cLgram.unigramN;
    auto trieWords = VStr();
    auto cbc = Candidate(); // currentBestCandidate
    auto currCandidates = Candidates();

    while (!input.empty()) {
        if (lgram == "") {
            cbc = findBestCandidateBySplitter_(input, toneless);

            if (cbc.dict_id == 0) {
                input.erase(0, 1);
                continue;
            }

            lgram = cbc.output;
            lgramCount = cbc.unigramN;

            auto i = 0;
            while (i < input.size() && cbc.ascii[i] == input[i]) {
                i++;
            }

            if (i < cbc.ascii.size()) {
                cbc.ascii.erase(cbc.ascii.begin() + i, cbc.ascii.end());
            }

            input.erase(0, i);
            rCandidates.push_back(cbc);
            continue;
        }

        trie_.getAllWords(input, toneless, trieWords);

        if (trieWords.empty()) {
            // Try the splitter again
            lgram.clear();
            lgramCount = 0;
            continue;
        }

        auto matched = VStr();

        std::copy_if(trieWords.begin(), trieWords.end(),
                     std::back_inserter(matched), [&](std::string word) {
                         auto i = 0;
                         while (i < input.size() && word[i] == input[i]) {
                             i++;
                         }

                         return splitter_.canSplit(
                             std::string(input.begin() + i, input.end()));
                     });

        db_.selectCandidatesFor(matched, currCandidates);

        if (currCandidates.empty()) {
            // Try the splitter again
            lgram.clear();
            lgramCount = 0;
            continue;
        }

        auto idx =
            findBestCandidateByBigram_(lgram, lgramCount, currCandidates);
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
        rCandidates.push_back(cbc);
    }

    return rCandidates;
}

} // namespace TaiKey

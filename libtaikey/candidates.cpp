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

// CandidateFinder::CandidateFinder(TKDB &db) : db_(db) {
//    auto syllables = db_.selectSyllableList();
//    splitter_ = Splitter(syllables);
//
//    auto dictionary = db_.selectTrieWordlist();
//    trie_ = Trie(dictionary);
//}

CandidateFinder::CandidateFinder(TKDB &db, Splitter &splitter, Trie &trie)
    : db_(db), splitter_(splitter), trie_(trie) {}

/**
 * Returns the number of `syllables` matched by `word`, starting
 * from the left; 0 if no syllables match
 */
size_t alignedSyllables(const VStr &syllables, std::string word) {
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

/**
 * Makes database call to look up lgram/rgram pairs, returns
 * index in `&rgrams` of the best match
 */
auto CandidateFinder::findBestCandidateByBigram_(std::string lgram,
                                                 int lgramCount,
                                                 const CandidateRows &rgrams)
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
                                                   bool toneless)
    -> CandidateRow {
    auto syllables = VStr();
    auto trieWords = VStr();
    auto candRows = CandidateRows();

    splitter_.split(input, syllables);
    trie_.getAllWords(input, toneless, trieWords);
    auto matched = VStr();
    std::copy_if(trieWords.begin(), trieWords.end(),
                 std::back_inserter(matched), [&](std::string word) {
                     return alignedSyllables(syllables, word) > 0;
                 });

    db_.selectCandidatesFor(matched, candRows);

    if (candRows.empty()) {
        return CandidateRow();
    }

    return candRows[0];
}

auto CandidateFinder::findCandidates(std::string input, bool toneless,
                                     std::string lgram, CandidateList &results)
    -> void {
    results.clear();

    if (input.empty()) {
        return;
    }

    int lgramCount = 0;

    if (!lgram.empty()) {
        lgramCount = db_.getUnigramCount(lgram);
    }

    // if (lgram == "") {
    //    auto cr = findBestCandidateBySplitter_(input, toneless);
    //    lgram = cr.output;
    //    lgramCount = cr.unigramN;
    //} else {
    //    lgramCount = db_.getUnigramCount(lgram);
    //}

    auto syllables = VStr();
    splitter_.split(input, syllables);
    auto sdeque = std::deque(syllables.cbegin(), syllables.cend());

    auto sylIterA = syllables.cbegin();
    auto sylIterB = syllables.cbegin();
    auto trieWords = VStr();
    auto remainder = ""s;
    auto prevBestCand = CandidateRow();
    auto currBestCand = CandidateRow();
    auto candRows = CandidateRows();
    auto bigrams = BigramWeights();

    while (!input.empty()) {
        if (lgram == "") {
            auto cr = findBestCandidateBySplitter_(input, toneless);
            lgram = cr.output;
            lgramCount = cr.unigramN;

            auto i = 0;
            while (i < input.size() && cr.ascii[i] == input[i]) {
                i++;
            }

            input.erase(0, i);
            results.push_back(Candidate{cr.ascii, cr.output, cr.hint});
            continue;
        }

        // remainder = boost::algorithm::join(sdeque, "");

        trie_.getAllWords(input, toneless, trieWords);

        if (trieWords.empty()) {
            // Try the splitter again
            lgram.clear();
            lgramCount = 0;
            continue;
        }

        auto matched = VStr();

        // auto matched = VStr();
        std::copy_if(trieWords.begin(), trieWords.end(),
                     std::back_inserter(matched), [&](std::string word) {
                         auto i = 0;
                         while (i < input.size() && word[i] == input[i]) {
                             i++;
                         }

                         return splitter_.canSplit(
                             std::string(input.begin() + i, input.end()));
                     });

        db_.selectCandidatesFor(matched, candRows);

        if (candRows.empty()) {
            // Try the splitter again
            lgram.clear();
            lgramCount = 0;
            continue;
        }

        auto idx = findBestCandidateByBigram_(lgram, lgramCount, candRows);
        currBestCand = candRows[idx];

        int i = 0;
        while (i < input.size() && currBestCand.ascii[i] == input[i]) {
            i++;
        }

        lgram = currBestCand.output;
        lgramCount = currBestCand.unigramN;

        results.push_back(Candidate{currBestCand.ascii, currBestCand.output,
                                    currBestCand.hint});
        input.erase(0, i);

        // auto consumed = alignedSyllables(sdeque, currBestCand.ascii);

        // results.push_back(Candidate{
        //    boost::algorithm::join(
        //        boost::iterator_range<std::deque<std::string>::const_iterator>(
        //            sdeque.cbegin(), sdeque.cbegin() + consumed),
        //        ""),
        //    currBestCand.output,
        //});

        // sdeque.erase(sdeque.cbegin(), sdeque.cbegin() + consumed);
    }

    return;
}

} // namespace TaiKey

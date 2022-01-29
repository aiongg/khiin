#include "CandidateFinder.h"

#include <algorithm>
#include <numeric>
#include <regex>
#include <string>

#include <boost/range/adaptor/indexed.hpp>

#include "Engine.h"
#include "Lomaji.h"
#include "Splitter.h"
#include "Trie.h"

namespace khiin::engine {

CandidateChunk::CandidateChunk(std::string raw) : raw(raw){};
CandidateChunk::CandidateChunk(std::string raw, Token token) : raw(raw), token(token){};

namespace {

using namespace std::literals::string_literals;

Candidate oneChunkCandidate(std::string &&raw, Token &&token) {
    return Candidate{CandidateChunk(std::move(raw), std::move(token))};
}

/**
 * Returns the number of `syllables` matched by `word`, starting
 * from the left; 0 if no syllables match
 */
size_t alignedSyllables(const string_vector &syllables, std::string word) {
    if (std::find(syllables.begin(), syllables.end(), word) != syllables.end()) {
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

// CandidateFinder

class CandidateFinderImpl : public CandidateFinder {
  public:
    CandidateFinderImpl(Engine *engine) : engine(engine) {
        database = engine->database();
        
    };

    virtual Candidates findCandidates(std::string input, std::string lgram, bool toneless) override {
        auto ret = Candidates();

        if (input.empty()) {
            return ret;
        }

        auto tokens = Tokens();
        auto lgram_count = lgram.empty() ? 0 : database->UnigramCount(lgram);
        auto trieWords = string_vector();

        word_trie->FindKeys(input, toneless, trieWords);

        if (trieWords.empty()) {
            // TODO
        }

        database->GetTokens(trieWords, tokens);

        if (lgram_count > 0) {
            sortTokensByBigram(lgram, lgram_count, tokens);
        }

        for (auto &t : tokens) {
            auto input_it = input.cbegin();
            auto input_end = input.cend();
            auto token_it = t.ascii.cbegin();
            auto token_end = t.ascii.cend();

            while (input_it != input_end && token_it != token_end && *input_it == *token_it) {
                ++input_it;
                ++token_it;
            }
            ret.push_back(oneChunkCandidate(std::string(input.cbegin(), input_it), std::move(t)));
        }

        return ret;
    }

    /**
     * Token::ascii matches next string on output, even if the
     * original result from the database included a tone number and
     * the next string didn't (fuzzy tone mode). That is, the ascii
     * member of a ret matches exactly the user's raw next and
     * can be directly compared with the raw buffer_ text.
     */
    virtual Candidate findPrimaryCandidate(std::string_view input, std::string lgram, bool fuzzy) override {
        auto ret = Candidate();

        if (input.empty()) {
            return ret;
        }

        auto lgramCount = lgram.empty() ? 0 : database->UnigramCount(lgram);

        auto bestToken = Token();
        auto dbTokens = Tokens();

        auto start = input.cbegin();
        auto end = input.cend();

        auto remainderFrom = [&](std::string_view::const_iterator &it) {
            return std::string(it, end);
        };
        auto remainder = [&]() {
            return remainderFrom(start);
        };

        auto hasKhinHyphens = false;
        auto trie_results = string_vector();
        trie_results.reserve(100);

        while (start != end) {
            if (handleHyphens(start, input.cend(), ret)) {
                continue;
            }

            word_trie->FindKeys(remainder(), fuzzy, trie_results);

            if (trie_results.empty()) {
                handleNoTrieMatch(start, input.cend(), ret, fuzzy);
                lgram.clear();
                lgramCount = 0;
                continue;
            }

            // default case: found stuff in the trie
            auto matched = string_vector();

            if (lgram.empty()) {
                auto splitResult = string_vector();
                // At the beginning, use the syllable splitter to get best result
                splitter->Split(remainder(), splitResult);

                std::copy_if(trie_results.begin(), trie_results.end(), std::back_inserter(matched),
                             [&](std::string word) {
                                 return word.rfind(splitResult[0], 0) == 0;
                             });
                if (matched.empty() && !splitResult.empty()) {
                    matched.push_back(std::move(splitResult[0]));
                }
            } else {
                // Otherwise, use the best fit leaving as few dangling characters
                // at the end as possible
                auto stopPos = 0;

                auto canSplit = [&](std::string word) {
                    auto input_it = std::string_view::const_iterator(start);
                    auto word_it = word.cbegin();

                    while (input_it != input.cend() && word_it != word.cend() && *input_it == *word_it) {
                        ++input_it;
                        ++word_it;
                    }

                    return splitter->CanSplit(std::string(input_it, input.cend() - stopPos));
                };

                while (matched.empty()) {
                    std::copy_if(trie_results.begin(), trie_results.end(), std::back_inserter(matched), canSplit);

                    if (matched.empty()) {
                        stopPos++;
                    }
                }
            }

            database->GetTokens(matched, dbTokens);

            if (dbTokens.empty() && !matched.empty()) {
                auto next = start;
                auto m_it = matched[0].cbegin();
                while (next != end && m_it != matched[0].cend() && *next == *m_it) {
                    next++;
                    m_it++;
                }
                ret.push_back(CandidateChunk(std::string(start, next)));
                start = next;
                continue;
            }

            auto idx = size_t(0);

            if (lgram != "") {
                idx = bestTokenByBigram(lgram, lgramCount, dbTokens);
            }

            bestToken = dbTokens[idx];

            auto next = start;
            auto t_it = bestToken.ascii.cbegin();
            auto t_end = bestToken.ascii.cend();

            while (next != end && t_it != t_end && *next == *t_it) {
                next++;
                t_it++;
            }

            lgram = bestToken.output;
            lgramCount = bestToken.unigramN;

            auto chunk = std::string(start, next);

            // handle khin
            if (ret.size() > 0 && ret.back().raw == "--" && ret.back().token.empty()) {
                ret.back().token = std::move(bestToken);
                ret.back().raw.append(chunk);
            } else {
                ret.push_back(CandidateChunk(std::string(start, next), std::move(bestToken)));
            }

            start = next;
        }

        return ret;
    }

  private:
    // modifies rgrams
    void sortTokensByBigram(std::string lgram, int lgramCount, Tokens &rgrams) {
        auto bigrams = BigramWeights();
        auto rgramOutputs = string_vector();
        for (const auto &c : rgrams) {
            rgramOutputs.push_back(c.output);
        }
        database->BigramsFor(lgram, rgramOutputs, bigrams);
        for (auto &c : rgrams) {
            c.bigramWt = static_cast<float>(bigrams.at(c.output)) / static_cast<float>(lgramCount);
        }
        std::sort(rgrams.begin(), rgrams.end(), [](Token a, Token b) -> bool {
            return a.bigramWt > b.bigramWt;
        });
    }

    /**
     * Makes database call to look up lgram/rgram pairs, returns
     * index in `&rgrams` of the best match
     */
    size_t bestTokenByBigram(std::string lgram, int lgramCount, const Tokens &rgrams) {
        if (lgramCount == 0) {
            return 0;
        }

        auto bigrams = BigramWeights();
        auto rgramOutputs = string_vector();
        for (const auto &c : rgrams) {
            rgramOutputs.push_back(c.output);
        }
        database->BigramsFor(lgram, rgramOutputs, bigrams);

        auto bestWeight = 0.0f;
        auto bestMatch = size_t(0);

        for (const auto &c : rgrams | boost::adaptors::indexed(0)) {
            auto weight = static_cast<float>(bigrams[c.value().output]) / static_cast<float>(lgramCount);
            if (weight > bestWeight) {
                bestMatch = c.index();
            }
        }

        return bestMatch;
    }

    Token findBestCandidateBySplitter_(std::string input, bool toneless) {
        auto syllables = string_vector();
        auto trieWords = string_vector();
        auto tokens = Tokens();

        splitter->Split(input, syllables);
        word_trie->FindKeys(input, toneless, trieWords);

        if (trieWords.size() == 0) {
            return Token{0, syllables[0], syllables[0], syllables[0]};
        }

        auto matched = string_vector();
        std::copy_if(trieWords.begin(), trieWords.end(), std::back_inserter(matched), [&](std::string word) {
            return alignedSyllables(syllables, word) > 0;
        });

        database->GetTokens(matched, tokens);

        if (tokens.empty()) {
            return Token{0, syllables[0], syllables[0], syllables[0]};
        }

        return tokens[0];
    }

    bool handleHyphens(std::string_view::const_iterator &start, const std::string_view::const_iterator &end,
                       Candidate &ret) {
        if (*start != '-') {
            return false;
        }

        auto next = start + 1;

        while (next != end && *next == '-') {
            ++next;
        }

        auto nHyphens = std::distance(start, next);

        if ((nHyphens == 1 || nHyphens == 3) && !ret.empty()) {
            // dangling hyphen on last chunk
            ret.back().raw += '-';
        }

        if (nHyphens == 2 || nHyphens == 3) {
            ret.push_back(CandidateChunk(std::string(2, '-')));
        }

        if (nHyphens > 3) {
            ret.push_back(CandidateChunk(std::string(nHyphens, '-')));
        }

        start = next;
        return true;
    }

    void handleNoTrieMatch(std::string_view::const_iterator &start, const std::string_view::const_iterator &end,
                           Candidate &candidate, bool fuzzy) {
        auto next = start + 1;
        auto trie_results = string_vector();

        // Collect next letters as long as they are also not
        // found in the Trie, put everything into this ret
        while (next != end) {
            word_trie->FindKeys(std::string(next, end), fuzzy, trie_results);
            if (trie_results.empty()) {
                ++next;
            } else {
                break;
            }
        }

        auto chunk = std::string(start, next);
        auto &prev = candidate.size() == 0 ? "" : candidate.back().raw;

        // If the collected letters are the whole remainder of the
        // string, and they form a prefix when combined with the existing
        // previous ret, join them together
        if (next == end && syl_trie->ContainsPrefix(prev + chunk) && candidate.size() > 0) {
            candidate.back().raw.append(chunk);
            candidate.back().token.clear();
        } else {
            candidate.push_back(CandidateChunk(std::move(chunk)));
        }

        start = next;
    }

    Database *database = nullptr;
    Splitter *splitter = nullptr;
    Trie *word_trie = nullptr;
    Trie *syl_trie = nullptr;
    Engine *engine = nullptr;
};

} // namespace

CandidateFinder *CandidateFinder::Create(Engine *engine) {
    return new CandidateFinderImpl(engine);
}

} // namespace khiin::engine

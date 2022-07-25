#include <cassert>

#include "CandidateFinder.h"

#include "config/KeyConfig.h"
#include "data/Database.h"
#include "data/Dictionary.h"
#include "data/Splitter.h"
#include "data/Trie.h"
#include "data/UserDictionary.h"

#include "Engine.h"
#include "Segmenter.h"
#include "SyllableParser.h"

namespace khiin::engine {

namespace {
using namespace unicode;

constexpr uint32_t kNumberOfContinuousCandidates = 5;

inline bool IsHigherFrequency(TaiToken *a, TaiToken *b) {
    return a->input_id == b->input_id ? a->weight > b->weight : a->input_id < b->input_id;
}

void LoadUnigramCounts(Engine *engine, std::vector<TokenResult> &options) {
    auto grams = std::vector<std::string>();
    std::for_each(std::begin(options), std::end(options), [&](TokenResult &option) {
        grams.push_back(option.token->output);
    });

    auto result = engine->database()->UnigramCounts(grams);

    for (auto &option : options) {
        auto it = std::find_if(result.cbegin(), result.cend(), [&](auto const &gram) {
            return gram.value == option.token->output;
        });
        if (it != result.cend()) {
            option.unigram_count = it->count;
        }
    }
}

void LoadBigramCounts(Engine *engine, TaiToken *lgram, std::vector<TokenResult> &options) {
    if (lgram == nullptr) {
        return;
    }

    auto rgrams = std::vector<std::string>();
    std::for_each(std::begin(options), std::end(options), [&](TokenResult &option) {
        rgrams.push_back(option.token->output);
    });
    auto result = engine->database()->BigramCounts(lgram->output, rgrams);

    for (auto &option : options) {
        auto it = std::find_if(result.cbegin(), result.cend(), [&](auto const &gram) {
            return gram.value == option.token->output;
        });
        if (it != result.cend()) {
            option.bigram_count = it->count;
        }
    }
}

bool CompareTokenResultsByLengthFirst(TokenResult const &a, TokenResult const &b) {
    if (a.input_size != b.input_size) {
        return a.input_size > b.input_size;
    }

    if (a.bigram_count != b.bigram_count) {
        return a.bigram_count > b.bigram_count;
    }

    if (a.unigram_count != b.unigram_count) {
        return a.unigram_count > b.unigram_count;
    }

    if (a.token->weight != b.token->weight) {
        return a.token->weight > b.token->weight;
    }

    return a.token->input_id < b.token->input_id;
}

inline void SortTokenResults(Engine *engine, TaiToken *lgram, std::vector<TokenResult> &options) {
    LoadUnigramCounts(engine, options);
    LoadBigramCounts(engine, lgram, options);
    std::sort(options.begin(), options.end(), CompareTokenResultsByLengthFirst);
}

inline void SortTokensByDefault(std::vector<TaiToken *> &tokens) {
    std::sort(tokens.begin(), tokens.end(), IsHigherFrequency);
}

TaiToken *BestMatchUnigram(Engine *engine, std::vector<TaiToken *> const &options) {
    return engine->database()->HighestUnigramCount(options);
}

TaiToken *BestMatchBigram(Engine *engine, TaiToken *lgram, std::vector<TaiToken *> const &options) {
    assert(lgram);
    return engine->database()->HighestBigramCount(lgram->output, options);
}

TaiToken *BestMatchNgram(Engine *engine, TaiToken *lgram, std::vector<TaiToken *> &options) {
    if (options.empty()) {
        return nullptr;
    }

    if (lgram != nullptr) {
        if (auto *ret = BestMatchBigram(engine, lgram, options); ret != nullptr) {
            return ret;
        }
    }

    if (auto *ret = BestMatchUnigram(engine, options); ret != nullptr) {
        return ret;
    }

    if (!options.empty()) {
        SortTokensByDefault(options);
        return options[0];
    }

    return nullptr;
}

void CopyTokenOptions(std::vector<TokenResult> &options, std::vector<TaiToken *> &tokens) {
    std::for_each(std::begin(options), std::end(options), [&](TokenResult &option) {
        tokens.push_back(option.token);
    });
}

TaiToken *BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->Autocomplete(copy_str_tolower(query));
    auto tokens = std::vector<TaiToken *>();
    CopyTokenOptions(options, tokens);
    return BestMatchNgram(engine, lgram, tokens);
}

TaiToken *BestSingleTokenMatch(Engine *engine, TaiToken *lgram, std::string const &input) {
    auto options = engine->dictionary()->WordSearch(copy_str_tolower(input));
    auto tokens = std::vector<TaiToken *>();
    CopyTokenOptions(options, tokens);
    return BestMatchNgram(engine, lgram, tokens);
}

std::vector<Buffer> TokensToBuffers(Engine *engine, std::vector<TokenResult> const &options, std::string const &query) {
    auto ret = std::vector<Buffer>();
    auto *parser = engine->syllable_parser();

    for (auto const &option : options) {
        auto elem = BufferElement::Build(parser, query.substr(0, option.input_size), option.token, true, true);
        ret.push_back(Buffer(std::move(elem)));
    }

    return ret;
}

std::set<size_t> InvalidSplitIndices(Engine *engine, std::string const &query) {
    auto ret = std::set<size_t>();

    if (query.size() <= 1) {
        return ret;
    }

    for (auto i = 1; i < query.size(); ++i) {
        if (engine->keyconfig()->IsToneKey(query.at(i))) {
            ret.insert(i);
        }
    }

    return ret;
}

void DedupeAndSortTokenResultSet(Engine *engine, TaiToken *lgram, std::string const &query,
                                 std::vector<TokenResult> &options) {
    auto *keyconfig = engine->keyconfig();
    auto seen = std::set<std::string>();
    auto invalid_sizes = InvalidSplitIndices(engine, query);

    auto it = options.begin();
    while (it != options.end()) {
        if (invalid_sizes.count(it->input_size) > 0) {
            it = options.erase(it);
            continue;
        }

        // Don't allow dangling tones
        //if (auto i = it->input_size; i < query.size() && keyconfig->IsToneKey(query[i])) {
        //    invalid_sizes.insert(it->input_size);
        //    it = options.erase(it);
        //    continue;
        //}

        if (!seen.insert(it->token->output).second) {
            it = options.erase(it);
            continue;
        }

        ++it;
    }

    SortTokenResults(engine, lgram, options);
}

std::vector<Buffer> AllWordsFromStart(Engine *engine, TaiToken *lgram, std::string const &query) {
    // auto ret = std::vector<Buffer>();
    auto query_lc = unicode::copy_str_tolower(query);
    auto options = engine->dictionary()->AllWordsFromStart(query_lc);

    DedupeAndSortTokenResultSet(engine, lgram, query, options);

    return TokensToBuffers(engine, options, query);
}

Buffer WordsToBuffer(Engine *engine, std::vector<std::string> const &words) {
    auto *parser = engine->syllable_parser();
    auto ret = Buffer();
    TaiToken *prev_best_match = nullptr;
    for (auto &word : words) {
        auto *best_match = BestSingleTokenMatch(engine, prev_best_match, word);
        ret.Append(BufferElement::Build(parser, word, best_match, true, true));
        prev_best_match = best_match;
    }
    return ret;
}

Punctuation OnePunctuation(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto puncts = engine->dictionary()->SearchPunctuation(query);
    if (!puncts.empty()) {
        return puncts[0];
    }
    return Punctuation{0, query, query, std::string()};
}

std::vector<Buffer> AllPunctuation(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto ret = std::vector<Buffer>();
    auto options = engine->dictionary()->SearchPunctuation(query);
    for (auto &p : options) {
        ret.push_back(Buffer(BufferElement(p)));
        ret.back().SetConverted(true);
    }
    return ret;
}

Buffer OneSplittable(Engine *engine, TaiToken *lgram, std::string_view query) {
    auto segmentations = engine->dictionary()->Segment(query, 1);

    if (segmentations.empty()) {
        return Buffer();
    }

    return WordsToBuffer(engine, segmentations[0]);
}

std::vector<Buffer> AllSplittables(Engine *engine, TaiToken *lgram, std::string const &query) {
    // auto all_cands = std::vector<Buffer>();
    auto segmentations = engine->dictionary()->Segment(query, kNumberOfContinuousCandidates);
    auto seen = std::unordered_set<std::string>();
    auto ret = std::vector<Buffer>();

    for (auto &segmentation : segmentations) {
        auto buf = WordsToBuffer(engine, segmentation);
        if (seen.insert(buf.Text()).second) {
            ret.push_back(std::move(buf));
        }
    }

    auto additionals = CandidateFinder::MultiMatch(engine, lgram, query);
    for (auto &addl : additionals) {
        if (seen.insert(addl.Text()).second) {
            ret.push_back(std::move(addl));
        }
    }

    return ret;
}

Buffer OneUserItem(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto *userdict = engine->user_dict();
    if (userdict != nullptr) {
        auto options = userdict->SearchExact(query);
        if (!options.empty()) {
            DedupeAndSortTokenResultSet(engine, lgram, query, options);
            if (options.size() > 1) {
                options.erase(options.begin() + 1);
            }
            auto ret = TokensToBuffers(engine, options, query);
            return ret[0];
        }
    }

    return Buffer(BufferElement(query));
}

std::vector<Buffer> AllUserItems(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto *userdict = engine->user_dict();
    if (userdict != nullptr) {
        auto options = userdict->Search(query);
        if (!options.empty()) {
            DedupeAndSortTokenResultSet(engine, lgram, query, options);
            return TokensToBuffers(engine, options, query);
        }
    }

    return std::vector<Buffer>();
}

// void AddUserItems(Engine *engine, TaiToken *lgram, std::string const &query, std::vector<Buffer> &candidates) {
//    auto items = AllUserItems(engine, lgram, query);
//    if (!items.empty()) {
//        candidates.insert(candidates.end(), items.begin(), items.end());
//    }
//}

} // namespace

std::vector<Buffer> CandidateFinder::MultiMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    if (query.empty()) {
        return std::vector<Buffer>();
    }

    auto invalid_split_indices = InvalidSplitIndices(engine, query);

    auto segment = Segmenter::LongestSegmentFromStart(engine, query);

    switch (segment.type) {
    case SegmentType::Punct:
        return AllPunctuation(engine, lgram, query);
    case SegmentType::Splittable:
        return AllWordsFromStart(engine, lgram, query);
    case SegmentType::UserItem:
        return AllUserItems(engine, lgram, query);
    case SegmentType::SyllablePrefix:
        return std::vector<Buffer>{
            Buffer(BufferElement::Build(engine->syllable_parser(), query, nullptr, false, true))};
    default: {
        auto ret = std::vector<Buffer>{Buffer(BufferElement(query))};
        ret.back().SetConverted(true);
        return ret;
    }
    }
}

Buffer CandidateFinder::ContinuousSingleMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto ret = Buffer();
    auto segments = Segmenter::SegmentText(engine, query);
    auto *parser = engine->syllable_parser();

    for (size_t i = 0; i < segments.size(); ++i) {
        auto &seg = segments[i];
        auto segment_raw = query.substr(seg.start, seg.size);
        auto *lgram_ = ret.Empty() ? lgram : ret.Back().candidate();

        switch (seg.type) {
        case SegmentType::Splittable:
            ret.Append(OneSplittable(engine, lgram_, segment_raw));
            break;
        case SegmentType::Punct:
            ret.Append(OnePunctuation(engine, lgram_, segment_raw));
            break;
        case SegmentType::UserItem:
            ret.Append(OneUserItem(engine, lgram_, segment_raw));
            break;
        case SegmentType::SyllablePrefix:
            ret.Append(parser, segment_raw);
            break;
        case SegmentType::WordPrefix: {
            auto *best_match = BestAutocomplete(engine, nullptr, segment_raw);
            if (best_match != nullptr) {
                ret.Append(parser, segment_raw, best_match);
            } else {
                ret.Append(parser, segment_raw);
            }
            break;
        }
        case SegmentType::Hyphens:
            ret.Append(std::string(seg.size, '-'));
            break;
        case SegmentType::None:
            ret.Append(std::move(segment_raw));
            break;
        }
    }

    return ret;
}

std::vector<Buffer> CandidateFinder::ContinuousMultiMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto candidates = std::vector<Buffer>{Buffer()};
    auto segments = Segmenter::SegmentText(engine, query);
    auto *parser = engine->syllable_parser();

    for (size_t i = 0; i < segments.size(); ++i) {
        auto &seg = segments[i];
        auto segment_raw = query.substr(seg.start, seg.size);
        auto *lgram_ = candidates.at(0).Empty() ? lgram : candidates.at(0).Back().candidate();

        switch (seg.type) {
        case SegmentType::Splittable:
            if (i == 0) {
                candidates = AllSplittables(engine, lgram_, segment_raw);
            } else {
                candidates[0].Append(OneSplittable(engine, lgram_, segment_raw));
            }
            break;
        case SegmentType::Punct:
            if (i == 0) {
                candidates = AllPunctuation(engine, lgram_, segment_raw);
            } else {
                candidates[0].Append(OnePunctuation(engine, lgram_, segment_raw));
            }
            break;
        case SegmentType::UserItem:
            if (i == 0) {
                candidates = AllUserItems(engine, lgram_, segment_raw);
            } else {
                candidates[0].Append(OneUserItem(engine, lgram_, segment_raw));
            }
            break;
        case SegmentType::SyllablePrefix:
            candidates[0].Append(parser, segment_raw);
            break;
        case SegmentType::WordPrefix: {
            auto *best_match = BestAutocomplete(engine, nullptr, segment_raw);
            if (best_match != nullptr) {
                candidates[0].Append(parser, segment_raw, best_match);
            } else {
                candidates[0].Append(parser, segment_raw);
            }
            break;
        }
        case SegmentType::Hyphens:
            candidates[0].Append(std::string(seg.size, '-'));
            break;
        case SegmentType::None:
            candidates[0].Append(std::move(segment_raw));
            break;
        }
    }

    candidates.at(0).SetConverted(true);

    return candidates;
}

bool CandidateFinder::HasExactMatch(Engine *engine, std::string_view query) {
    return engine->dictionary()->word_splitter()->CanSplit(query);
}

} // namespace khiin::engine

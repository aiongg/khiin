
#include "CandidateFinder.h"

#include "Database.h"
#include "Dictionary.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "Segmenter.h"
#include "Splitter.h"
#include "SyllableParser.h"
#include "Trie.h"

namespace khiin::engine {

namespace {
using namespace unicode;

constexpr uint32_t kNumberOfContinuousCandidates = 5;

inline bool IsHigherFrequency(TaiToken *a, TaiToken *b) {
    return a->input_id == b->input_id ? a->weight > b->weight : a->input_id < b->input_id;
}

void LoadUnigramCounts(Engine *engine, std::vector<TokenResult> &options) {
    auto grams = std::vector<std::string>();
    for (auto const &option : options) {
        grams.push_back(option.token->output);
    }
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
    for (auto const &option : options) {
        rgrams.push_back(option.token->output);
    }
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

// bool CompareTokenResultsByWeightFirst(TokenResult const &a, TokenResult const &b) {
//    if (a.bigram_count != b.bigram_count) {
//        return a.bigram_count > b.bigram_count;
//    }
//
//    if (a.unigram_count != b.unigram_count) {
//        return a.unigram_count > b.unigram_count;
//    }
//
//    if (a.token->weight != b.token->weight) {
//        return a.token->weight > b.token->weight;
//    }
//
//    return a.token->input_id < b.token->input_id;
//}

// inline void SortTokenResultsByWeightFirst(Engine *engine, std::vector<TokenResult> &options) {
//    LoadUnigramCounts(engine, options);
//    std::sort(options.begin(), options.end(), CompareTokenResultsByWeightFirst);
//}

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

inline void SortTokenResultsByLengthFirst(Engine *engine, TaiToken *lgram, std::vector<TokenResult> &options) {
    LoadUnigramCounts(engine, options);
    LoadBigramCounts(engine, lgram, options);
    std::sort(options.begin(), options.end(), CompareTokenResultsByLengthFirst);
}

inline void SortTokensByFrequency(std::vector<TaiToken *> &tokens) {
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
        SortTokensByFrequency(options);
        return options[0];
    }

    return nullptr;
}

TaiToken *BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->Autocomplete(copy_str_tolower(query));
    auto tokens = std::vector<TaiToken *>();
    for (auto &opt : options) {
        tokens.push_back(opt.token);
    }
    return BestMatchNgram(engine, lgram, tokens);
}

TaiToken *BestSingleTokenMatch(Engine *engine, TaiToken *lgram, std::string const &input) {
    auto options = engine->dictionary()->WordSearch(copy_str_tolower(input));
    auto tokens = std::vector<TaiToken *>();
    for (auto &opt : options) {
        tokens.push_back(opt.token);
    }
    return BestMatchNgram(engine, lgram, tokens);
}

std::vector<Buffer> TokensToBuffers(Engine *engine, std::vector<TokenResult> const &options, std::string const &query) {
    auto ret = std::vector<Buffer>();
    auto *parser = engine->syllable_parser();

    for (auto const &option : options) {
        auto elem = BufferElement::Build(parser, query.substr(0, option.input_size), option.token, true, true);
        ret.push_back(std::move(elem));
    }

    return ret;
}

std::vector<Buffer> AllSplittableCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &raw_query) {
    auto ret = std::vector<Buffer>();
    auto query_lc = unicode::copy_str_tolower(raw_query);
    auto options = engine->dictionary()->AllWordsFromStart(query_lc);

    auto *splitter = engine->dictionary()->word_splitter();
    auto seen = std::set<std::string>();
    auto unsplittable = std::set<size_t>();

    auto it = options.begin();
    while (it != options.end()) {
        if (unsplittable.count(it->input_size) == 1) {
            it = options.erase(it);
            continue;
        }

        if (!splitter->CanSplit(raw_query.substr(it->input_size, raw_query.size()))) {
            unsplittable.insert(it->input_size);
            it = options.erase(it);
            continue;
        }

        if (!seen.insert(it->token->output).second) {
            it = options.erase(it);
            continue;
        }

        ++it;
    }

    SortTokenResultsByLengthFirst(engine, lgram, options);
    return TokensToBuffers(engine, options, raw_query);
}

std::vector<Buffer> AllPunctuation(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto ret = std::vector<Buffer>();
    auto options = engine->dictionary()->SearchPunctuation(query);
    for (auto &p : options) {
        ret.push_back(Buffer(p));
        ret.back().SetConverted(true);
    }
    return ret;
}

std::vector<Buffer> AllWordsFromStart(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto ret = std::vector<Buffer>();
    auto query_lc = unicode::copy_str_tolower(query);
    auto options = engine->dictionary()->AllWordsFromStart(query_lc);

    auto *keyconfig = engine->keyconfig();
    auto seen = std::set<std::string>();
    auto invalid_sizes = std::set<size_t>();

    auto it = options.begin();
    while (it != options.end()) {
        if (invalid_sizes.count(it->input_size) > 0) {
            it = options.erase(it);
            continue;
        }

        // Don't allow dangling tones
        if (auto i = it->input_size; i < query.size() && keyconfig->IsToneKey(query[i])) {
            invalid_sizes.insert(it->input_size);
            it = options.erase(it);
            continue;
        }

        if (!seen.insert(it->token->output).second) {
            it = options.erase(it);
            continue;
        }

        ++it;
    }

    SortTokenResultsByLengthFirst(engine, lgram, options);
    return TokensToBuffers(engine, options, query);
}

Buffer WordsToBuffer(Engine *engine, std::vector<std::string> &words) {
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

Punctuation BestPunctuation(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto puncts = engine->dictionary()->SearchPunctuation(query);
    if (!puncts.empty()) {
        return puncts[0];
    }
    return Punctuation{0, query, query, std::string()};
}

std::vector<Buffer> SplittableContinuousMultiMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto all_cands = std::vector<Buffer>();
    auto segmentations = engine->dictionary()->Segment(query, kNumberOfContinuousCandidates);
    auto seen = std::unordered_set<std::string>();
    auto ret = std::vector<Buffer>();

    for (auto &segmentation : segmentations) {
        auto buf = WordsToBuffer(engine, segmentation);
        if (seen.insert(buf.Text()).second) {
            ret.push_back(std::move(buf));
        }
    }

    auto additionals = CandidateFinder::WordsByWeight(engine, lgram, query);
    for (auto &addl : additionals) {
        if (seen.insert(addl.Text()).second) {
            ret.push_back(std::move(addl));
        }
    }

    return ret;
}

} // namespace

std::vector<Buffer> CandidateFinder::WordsByWeight(Engine *engine, TaiToken *lgram, std::string const &raw_query) {
    auto ret = std::vector<Buffer>();
    if (raw_query.empty()) {
        return ret;
    }

    auto segments = Segmenter::SegmentText(engine, raw_query);

    if (segments.empty()) {
        ret.push_back(Buffer(raw_query));
        ret.back().SetConverted(true);
        return ret;
    }

    auto &first_seg = segments.at(0);
    auto query = raw_query.substr(0, first_seg.size);

    if (first_seg.type == SegmentType::Splittable) {
        return AllSplittableCandidatesFromStart(engine, lgram, query);
    }

    if (first_seg.type == SegmentType::Punct) {
        return AllPunctuation(engine, lgram, query);
    }

    ret.push_back(Buffer(query));
    ret.back().SetConverted(true);

    return ret;
}

std::vector<Buffer> CandidateFinder::WordsByLength(Engine *engine, TaiToken *lgram, std::string const &query) {
    if (query.empty()) {
        return std::vector<Buffer>();
    }

    auto segment = Segmenter::LongestSegmentFromStart(engine, query);

    switch (segment.type) {
    case SegmentType::Punct:
        return AllPunctuation(engine, lgram, query);
        break;
    case SegmentType::Splittable:
        return AllWordsFromStart(engine, lgram, query);
        break;
    default: {
        auto ret = std::vector<Buffer>{Buffer(query)};
        ret.back().SetConverted(true);
        return ret;
    }
    }
}

Buffer CandidateFinder::ContinuousBestMatch(Engine *engine, TaiToken *lgram, std::string_view query) {
    auto segmentations = engine->dictionary()->Segment(query, 1);

    if (segmentations.empty()) {
        return Buffer();
    }

    return WordsToBuffer(engine, segmentations[0]);
}

std::vector<Buffer> CandidateFinder::ContinuousMultiMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto candidates = std::vector<Buffer>{Buffer()};
    auto segments = Segmenter::SegmentText(engine, query);

    for (size_t i = 0; i < segments.size(); ++i) {
        auto &seg = segments[i];
        auto seg_raw_comp = query.substr(seg.start, seg.size);

        auto *lgram = candidates.at(0).Empty() ? nullptr : candidates.at(0).Back().candidate();

        switch (seg.type) {
        case SegmentType::Splittable:
            if (i == 0) {
                candidates = SplittableContinuousMultiMatch(engine, lgram, seg_raw_comp);
            } else {
                candidates[0].Append(ContinuousBestMatch(engine, lgram, seg_raw_comp));
            }
            break;
        case SegmentType::Hyphens:
            candidates[0].Append(std::string(seg.size, '-'));
            break;
        case SegmentType::Punct:
            if (i == 0) {
                candidates = AllPunctuation(engine, lgram, seg_raw_comp);
            } else {
                candidates[0].Append(BestPunctuation(engine, lgram, seg_raw_comp));
            }
            break;
        case SegmentType::None:
            candidates[0].Append(std::move(seg_raw_comp));
            break;
        case SegmentType::SyllablePrefix:
            candidates[0].Append(TaiText::FromRawSyllable(engine->syllable_parser(), seg_raw_comp));
            break;
        case SegmentType::WordPrefix:
            auto *best_match = BestAutocomplete(engine, nullptr, seg_raw_comp);
            if (best_match != nullptr) {
                candidates[0].Append(TaiText::FromMatching(engine->syllable_parser(), seg_raw_comp, best_match));
            } else {
                candidates[0].Append(TaiText::FromRawSyllable(engine->syllable_parser(), seg_raw_comp));
            }
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

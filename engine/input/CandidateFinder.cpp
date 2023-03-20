#include "CandidateFinder.h"

#include <cassert>

#include "Engine.h"
#include "Segmenter.h"
#include "SyllableParser.h"
#include "config/KeyConfig.h"
#include "data/Database.h"
#include "data/Dictionary.h"
#include "data/Splitter.h"
#include "data/Trie.h"
#include "data/UserDictionary.h"

namespace khiin::engine {

namespace {
using namespace unicode;

constexpr uint32_t kNumberOfContinuousCandidates = 5;

inline bool IsHigherFrequency(TaiToken const& a, TaiToken const& b) {
    return a.input_id == b.input_id ? a.weight > b.weight
                                    : a.input_id < b.input_id;
}

bool CompareTokenResultsByLengthFirst(TaiToken const& a, TaiToken const& b) {
    if (a.input_size != b.input_size) {
        return a.input_size > b.input_size;
    }

    if (a.bigram_count != b.bigram_count) {
        return a.bigram_count > b.bigram_count;
    }

    if (a.unigram_count != b.unigram_count) {
        return a.unigram_count > b.unigram_count;
    }

    if (a.weight != b.weight) {
        return a.weight > b.weight;
    }

    return a.input_id < b.input_id;
}

inline void SortTokenResults(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::vector<TaiToken>& options) {
    std::optional<std::string> lgram_str = std::nullopt;
    if (lgram) {
        lgram_str = lgram->output;
    }
    engine->database()->AddNGramsData(lgram_str, options);
    std::sort(options.begin(), options.end(), CompareTokenResultsByLengthFirst);
}

inline void SortTokensByDefault(std::vector<TaiToken>& tokens) {
    if (tokens.empty()) {
        return;
    }
    std::sort(tokens.begin(), tokens.end(), IsHigherFrequency);
}

std::optional<TaiToken> BestMatchNgram(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::vector<TaiToken>& options) {
    if (options.empty()) {
        return std::nullopt;
    }

    std::optional<std::string> lgram_str;

    if (lgram) {
        lgram_str = lgram.value().output;
    }

    engine->database()->AddNGramsData(lgram_str, options);

    SortTokensByDefault(options);
    return options[0];
}

std::optional<TaiToken> BestAutocomplete(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto options = engine->dictionary()->Autocomplete(copy_str_tolower(query));
    return BestMatchNgram(engine, lgram, options);
}

std::optional<TaiToken> BestSingleTokenMatch(
    Engine* engine, std::optional<TaiToken> lgram, std::string const& input) {
    auto options = engine->dictionary()->WordSearch(copy_str_tolower(input));
    return BestMatchNgram(engine, lgram, options);
}

std::vector<Buffer> TokensToBuffers(
    Engine* engine, std::vector<TaiToken> const& options,
    std::string const& query) {
    auto ret = std::vector<Buffer>();
    auto* parser = engine->syllable_parser();

    for (auto const& option : options) {
        auto elem = BufferElement::Builder()
                        .Parser(parser)
                        .FromInput(query.substr(0, option.input_size))
                        .WithTaiToken(option)
                        .SetConverted()
                        .SetCandidate()
                        .Build();

        ret.push_back(Buffer(std::move(elem)));
    }

    return ret;
}

std::set<size_t> InvalidSplitIndices(Engine* engine, std::string const& query) {
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

void DedupeAndSortTokenResultSet(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query, std::vector<TaiToken>& options) {
    //auto* keyconfig = engine->keyconfig();
    auto seen = std::set<std::string>();
    auto invalid_sizes = InvalidSplitIndices(engine, query);

    auto it = options.begin();
    while (it != options.end()) {
        if (invalid_sizes.count(it->input_size) > 0) {
            it = options.erase(it);
            continue;
        }

        // Don't allow dangling tones
        // if (auto i = it->input_size; i < query.size() &&
        // keyconfig->IsToneKey(query[i])) {
        //    invalid_sizes.insert(it->input_size);
        //    it = options.erase(it);
        //    continue;
        //}

        if (!seen.insert(it->output).second) {
            it = options.erase(it);
            continue;
        }

        ++it;
    }

    SortTokenResults(engine, lgram, options);
}

std::vector<Buffer> AllWordsFromStart(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    // auto ret = std::vector<Buffer>();
    auto query_lc = unicode::copy_str_tolower(query);
    auto options = engine->dictionary()->AllWordsFromStart(query_lc);

    DedupeAndSortTokenResultSet(engine, lgram, query, options);

    return TokensToBuffers(engine, options, query);
}

Buffer WordsToBuffer(Engine* engine, std::vector<std::string> const& words) {
    auto* parser = engine->syllable_parser();
    auto ret = Buffer();
    std::optional<TaiToken> prev_best_match = std::nullopt;
    for (auto const& word : words) {
        auto best_match = BestSingleTokenMatch(engine, prev_best_match, word);
        auto elem = BufferElement::Builder()
                        .Parser(parser)
                        .FromInput(word)
                        .SetCandidate()
                        .SetConverted();

        if (best_match) {
            elem.WithTaiToken(best_match.value());
        }

        ret.Append(elem.Build());

        prev_best_match = best_match;
    }
    return ret;
}

Punctuation OnePunctuation(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto puncts = engine->dictionary()->SearchPunctuation(query);
    if (!puncts.empty()) {
        return puncts[0];
    }
    return Punctuation{0, query, query, std::string()};
}

std::vector<Buffer> AllPunctuation(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto ret = std::vector<Buffer>();
    auto options = engine->dictionary()->SearchPunctuation(query);
    for (auto& p : options) {
        auto elem =
            BufferElement::Builder().WithPunctuation(p).SetConverted().Build();
        ret.push_back(Buffer(std::move(elem)));
    }
    return ret;
}

Buffer OneSplittable(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string_view query) {
    auto segmentations = engine->dictionary()->Segment(query, 1);

    if (segmentations.empty()) {
        return Buffer();
    }

    return WordsToBuffer(engine, segmentations[0]);
}

std::vector<Buffer> AllSplittables(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    // auto all_cands = std::vector<Buffer>();
    auto segmentations =
        engine->dictionary()->Segment(query, kNumberOfContinuousCandidates);
    auto seen = std::unordered_set<std::string>();
    auto ret = std::vector<Buffer>();

    for (auto& segmentation : segmentations) {
        auto buf = WordsToBuffer(engine, segmentation);
        if (seen.insert(buf.Text()).second) {
            ret.push_back(std::move(buf));
        }
    }

    auto additionals = CandidateFinder::MultiMatch(engine, lgram, query);
    for (auto& addl : additionals) {
        if (seen.insert(addl.Text()).second) {
            ret.push_back(std::move(addl));
        }
    }

    return ret;
}

Buffer OneUserItem(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto* userdict = engine->user_dict();
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

    auto elem = BufferElement::Builder().FromInput(query).Build();
    return Buffer(std::move(elem));
}

std::vector<Buffer> AllUserItems(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto* userdict = engine->user_dict();
    if (userdict != nullptr) {
        auto options = userdict->Search(query);
        if (!options.empty()) {
            DedupeAndSortTokenResultSet(engine, lgram, query, options);
            return TokensToBuffers(engine, options, query);
        }
    }

    return std::vector<Buffer>();
}

// void AddUserItems(Engine *engine, TaiToken *lgram, std::string const &query,
// std::vector<Buffer> &candidates) {
//    auto items = AllUserItems(engine, lgram, query);
//    if (!items.empty()) {
//        candidates.insert(candidates.end(), items.begin(), items.end());
//    }
//}

}  // namespace

std::vector<Buffer> BufferListOf(BufferElement&& elem) {
    return std::vector<Buffer>{Buffer(std::move(elem))};
}

std::vector<Buffer> CandidateFinder::MultiMatch(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
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
            return BufferListOf(BufferElement::Builder()
                                    .Parser(engine->syllable_parser())
                                    .FromInput(query)
                                    .SetConverted()
                                    .Build());
        default: {
            return BufferListOf(BufferElement::Builder()
                                    .FromInput(query)
                                    .SetConverted()
                                    .Build());
        }
    }
}

Buffer CandidateFinder::ContinuousSingleMatch(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto ret = Buffer();
    auto segments = Segmenter::SegmentText(engine, query);
    auto* parser = engine->syllable_parser();

    for (size_t i = 0; i < segments.size(); ++i) {
        auto& seg = segments[i];
        auto segment_raw = query.substr(seg.start, seg.size);
        auto const& lgram_ = ret.Empty() ? lgram : ret.Back().candidate();

        switch (seg.type) {
            case SegmentType::Splittable:
                ret.Append(OneSplittable(engine, lgram_, segment_raw));
                break;
            case SegmentType::Punct:
                ret.Append(BufferElement::Builder()
                               .WithPunctuation(
                                   OnePunctuation(engine, lgram_, segment_raw))
                               .Build());
                break;
            case SegmentType::UserItem:
                ret.Append(OneUserItem(engine, lgram_, segment_raw));
                break;
            case SegmentType::SyllablePrefix:
                ret.Append(BufferElement::Builder()
                               .Parser(parser)
                               .FromInput(segment_raw)
                               .Build());
                break;
            case SegmentType::WordPrefix: {
                auto best_match =
                    BestAutocomplete(engine, std::nullopt, segment_raw);
                auto builder =
                    BufferElement::Builder().Parser(parser).FromInput(
                        segment_raw);

                if (best_match) {
                    builder.WithTaiToken(best_match.value());
                }
                ret.Append(builder.Build());
                break;
            }
            case SegmentType::Hyphens:
                ret.Append(BufferElement::Builder()
                               .FromInput(std::string(seg.size, '-'))
                               .Build());
                break;
            case SegmentType::None:
                ret.Append(
                    BufferElement::Builder().FromInput(segment_raw).Build());
                break;
        }
    }

    return ret;
}

std::vector<Buffer> CandidateFinder::ContinuousMultiMatch(
    Engine* engine, std::optional<TaiToken> const& lgram,
    std::string const& query) {
    auto candidates = std::vector<Buffer>{Buffer()};
    auto segments = Segmenter::SegmentText(engine, query);
    auto* parser = engine->syllable_parser();

    for (size_t i = 0; i < segments.size(); ++i) {
        auto& seg = segments[i];
        auto segment_raw = query.substr(seg.start, seg.size);
        auto const& lgram_ = candidates.at(0).Empty()
                                 ? lgram
                                 : candidates.at(0).Back().candidate();

        switch (seg.type) {
            case SegmentType::Splittable:
                if (i == 0) {
                    candidates = AllSplittables(engine, lgram_, segment_raw);
                } else {
                    candidates[0].Append(
                        OneSplittable(engine, lgram_, segment_raw));
                }
                break;
            case SegmentType::Punct:
                if (i == 0) {
                    candidates = AllPunctuation(engine, lgram_, segment_raw);
                } else {
                    candidates[0].Append(BufferElement::Builder()
                                             .WithPunctuation(OnePunctuation(
                                                 engine, lgram_, segment_raw))
                                             .Build());
                }
                break;
            case SegmentType::UserItem:
                if (i == 0) {
                    candidates = AllUserItems(engine, lgram_, segment_raw);
                } else {
                    candidates[0].Append(
                        OneUserItem(engine, lgram_, segment_raw));
                }
                break;
            case SegmentType::SyllablePrefix:
                candidates[0].Append(BufferElement::Builder()
                                         .Parser(parser)
                                         .FromInput(segment_raw)
                                         .Build());
                break;
            case SegmentType::WordPrefix: {
                auto best_match =
                    BestAutocomplete(engine, std::nullopt, segment_raw);
                if (best_match) {
                    candidates[0].Append(BufferElement::Builder()
                                             .Parser(parser)
                                             .FromInput(segment_raw)
                                             .WithTaiToken(best_match.value())
                                             .Build());
                } else {
                    candidates[0].Append(BufferElement::Builder()
                                             .Parser(parser)
                                             .FromInput(segment_raw)
                                             .Build());
                }
                break;
            }
            case SegmentType::Hyphens:
                candidates[0].Append(BufferElement::Builder()
                                         .FromInput(std::string(seg.size, '-'))
                                         .Build());
                break;
            case SegmentType::None:
                candidates[0].Append(
                    BufferElement::Builder().FromInput(segment_raw).Build());
                break;
        }
    }

    candidates.at(0).SetConverted(true);

    return candidates;
}

bool CandidateFinder::HasExactMatch(Engine* engine, std::string_view query) {
    return engine->dictionary()->word_splitter()->CanSplit(query);
}

}  // namespace khiin::engine

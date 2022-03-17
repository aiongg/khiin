#include "CandidateFinder.h"

#include "Database.h"
#include "Dictionary.h"
#include "Engine.h"
#include "Segmenter.h"
#include "Splitter.h"
#include "SyllableParser.h"
#include "Trie.h"

namespace khiin::engine {

namespace {
using namespace unicode;

inline bool IsHigherFrequency(TaiToken *a, TaiToken *b) {
    return a->input_id == b->input_id ? a->weight > b->weight : a->input_id < b->input_id;
}

inline bool SizeAndFrequencySort(TaiToken *a, TaiToken *b) {

    auto a_size = letter_count(a->input);
    auto b_size = letter_count(b->input);

    return a_size == b_size ? IsHigherFrequency(a, b) : a_size > b_size;
}

inline void SortTokenResults(std::vector<TokenResult> &tokens) {
    std::sort(tokens.begin(), tokens.end(), [&](TokenResult const &a, TokenResult const &b) {
        return a.input_size == b.input_size
                   ? (a.token->input_id == b.token->input_id ? a.token->weight > b.token->weight
                                                             : a.token->input_id < b.token->input_id)
                   : a.input_size > b.input_size;
    });
}

inline void SortTokensByFrequency(std::vector<TaiToken *> &tokens) {
    std::sort(tokens.begin(), tokens.end(), IsHigherFrequency);
}

inline void SortTokensByLetterCountAndFrequency(std::vector<TaiToken *> &tokens) {
    std::sort(tokens.begin(), tokens.end(), SizeAndFrequencySort);
}

std::string RemoveRawFromStart(TaiText const &text, std::string const &raw_query) {
    auto ret = std::string();
    auto prefix = text.RawText();
}

TaiToken *BestMatchUnigram(Engine *engine, std::vector<TaiToken *> const &options) {
    return engine->database()->HighestUnigramCount(options);
}

TaiToken *BestMatchBigram(Engine *engine, TaiToken *lgram, std::vector<TaiToken *> const &options) {
    assert(lgram);
    return engine->database()->HighestBigramCount(lgram->output, options);
}

TaiToken *BestMatchImpl(Engine *engine, TaiToken *lgram, std::vector<TaiToken *> &options) {
    if (options.empty()) {
        return nullptr;
    }

    if (lgram) {
        auto ret = BestMatchBigram(engine, lgram, options);
        if (ret) {
            return ret;
        }
    }

    auto ret = BestMatchUnigram(engine, options);
    if (ret) {
        return ret;
    }

    if (options.size()) {
        SortTokensByFrequency(options);
        return options[0];
    }

    return nullptr;
}

bool LeavesGoodRemainder(Dictionary *dict, TaiText const &prefix, std::string_view raw_query) {
    auto size = prefix.RawSize();
    return dict->word_splitter()->CanSplit(raw_query.substr(size, raw_query.size() - size));
}

std::vector<Buffer> AllSplittableCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &raw_query) {
    auto ret = std::vector<Buffer>();
    auto query_lc = unicode::copy_str_tolower(raw_query);
    auto options = engine->dictionary()->AllWordsFromStart(query_lc);

    SortTokenResults(options);
    auto parser = engine->syllable_parser();
    auto seen = std::unordered_set<std::string>();

    for (auto &option : options) {
        if (!seen.insert(option.token->output).second) {
            continue;
        }

        if (!engine->dictionary()->word_splitter()->CanSplit(raw_query.substr(option.input_size, raw_query.size()))) {
            continue;
        }

        auto tai_text = parser->AsTaiText(raw_query.substr(0, option.input_size), option.token->input);
        tai_text.SetCandidate(option.token);
        auto elem = BufferElement(std::move(tai_text));
        elem.is_converted = true;
        ret.push_back(std::vector<BufferElement>{std::move(elem)});
    }

    return ret;
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

std::vector<Buffer> AllWordsFromStart(Engine* engine, TaiToken* lgram, std::string const& query) {
    auto ret = std::vector<Buffer>();
    auto query_lc = unicode::copy_str_tolower(query);
    auto options = engine->dictionary()->AllWordsFromStart(query_lc);
    SortTokenResults(options);

    auto parser = engine->syllable_parser();
    auto seen = std::unordered_set<std::string>();

    for (auto &option : options) {
        if (!seen.insert(option.token->output).second) {
            continue;
        }

        auto tai_text = parser->AsTaiText(query.substr(0, option.input_size), option.token->input);
        tai_text.SetCandidate(option.token);
        auto elem = BufferElement(std::move(tai_text));
        elem.is_converted = true;
        ret.push_back(std::vector<BufferElement>{std::move(elem)});
    }

    return ret;
}

std::vector<Buffer> LongestCandidatesFromStartImpl(Engine *engine, TaiToken *lgram, std::string const &raw_query) {
    if (raw_query.empty()) {
        return std::vector<Buffer>();
    }

    auto segment = Segmenter::LongestSegmentFromStart(engine, raw_query);

    switch (segment.type) {
    case SegmentType::Punct:
        return AllPunctuation(engine, lgram, raw_query);
        break;
    case SegmentType::Splittable:
        return AllWordsFromStart(engine, lgram, raw_query);
        break;
    default: {
        auto ret = std::vector<Buffer>{Buffer(raw_query)};
        ret.back().SetConverted(true);
        return ret;
    }
    }
}

std::vector<Buffer> GetCandidatesFromStartImpl(Engine *engine, TaiToken *lgram, std::string const &raw_query) {

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

Buffer WordsToBuffer(Engine *engine, std::vector<std::string> &words) {
    auto elems = BufferElementList();
    TaiToken *prev_best_match = nullptr;
    for (auto &word : words) {
        auto best_match = CandidateFinder::BestMatch(engine, prev_best_match, word);
        auto tai_text = TaiText::FromMatching(engine->syllable_parser(), word, best_match);
        tai_text.SetCandidate(best_match);
        auto elem = BufferElement(std::move(tai_text));
        elem.is_converted = true;
        elems.push_back(std::move(elem));
        prev_best_match = best_match;
    }
    return Buffer(std::move(elems));
}

Buffer BestContinuousCandidate(Engine *engine, TaiToken *lgram, std::string_view query) {
    auto segmentations = engine->dictionary()->Segment(query, 1);

    if (segmentations.empty()) {
        return Buffer();
    }

    return WordsToBuffer(engine, segmentations[0]);
}

std::vector<Buffer> AllContinuousCandidates(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto all_cands = std::vector<Buffer>();
    auto segmentations = engine->dictionary()->Segment(query, 5);
    auto seen = std::unordered_set<std::string>();
    auto ret = std::vector<Buffer>();

    for (auto &segmentation : segmentations) {
        auto buf = WordsToBuffer(engine, segmentation);
        if (seen.insert(buf.Text()).second) {
            ret.push_back(std::move(buf));
        }
    }

    auto additionals = GetCandidatesFromStartImpl(engine, lgram, query);
    for (auto &addl : additionals) {
        if (seen.insert(addl.Text()).second) {
            ret.push_back(std::move(addl));
        }
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

std::vector<Buffer> MultiSegmentCandidatesImpl(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto candidates = std::vector<Buffer>{Buffer()};
    auto segments = Segmenter::SegmentText(engine, query);

    for (auto i = 0; i < segments.size(); ++i) {
        auto &seg = segments[i];
        auto seg_raw_comp = query.substr(seg.start, seg.size);

        auto lgram = candidates.at(0).Empty() ? nullptr : candidates.at(0).Back().candidate();

        switch (seg.type) {
        case SegmentType::Splittable:
            if (i == 0) {
                candidates = AllContinuousCandidates(engine, lgram, seg_raw_comp);
            } else {
                candidates[0].Append(BestContinuousCandidate(engine, lgram, seg_raw_comp));
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
            auto best_match = CandidateFinder::BestAutocomplete(engine, nullptr, seg_raw_comp);
            if (best_match) {
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

} // namespace

TaiToken *CandidateFinder::BestMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->WordSearch(copy_str_tolower(query));
    auto tokens = std::vector<TaiToken *>();
    for (auto &opt : options) {
        tokens.push_back(opt.token);
    }
    return BestMatchImpl(engine, lgram, tokens);
}

TaiToken *CandidateFinder::BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->Autocomplete(copy_str_tolower(query));
    auto tokens = std::vector<TaiToken *>();
    for (auto &opt : options) {
        tokens.push_back(opt.token);
    }
    return BestMatchImpl(engine, lgram, tokens);
}

std::vector<Buffer> CandidateFinder::GetCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &query) {
    return GetCandidatesFromStartImpl(engine, lgram, query);
}

std::vector<Buffer> CandidateFinder::LongestCandidatesFromStart(Engine *engine, TaiToken *lgram,
                                                                std::string const &query) {
    return LongestCandidatesFromStartImpl(engine, lgram, query);
}

std::vector<Buffer> CandidateFinder::ContinuousCandidates(Engine *engine, TaiToken *lgram, std::string const &query) {
    return AllContinuousCandidates(engine, lgram, query);
}

std::vector<Buffer> CandidateFinder::MultiSegmentCandidates(Engine *engine, TaiToken *lgram, std::string const &query) {
    return MultiSegmentCandidatesImpl(engine, lgram, query);
}

Buffer CandidateFinder::ContinuousBestMatch(Engine *engine, TaiToken *lgram, std::string_view query) {
    return BestContinuousCandidate(engine, lgram, query);
}

bool CandidateFinder::HasExactMatch(Engine *engine, std::string_view query) {
    return engine->dictionary()->word_splitter()->CanSplit(query);
}

} // namespace khiin::engine

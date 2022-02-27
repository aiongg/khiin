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

inline bool IsHigherFrequency(TaiToken *left, TaiToken *right) {
    return left->input_id == right->input_id ? left->weight > right->weight : left->input_id < right->input_id;
}

inline void SortTokens(std::vector<TaiToken *> &tokens) {
    std::sort(tokens.begin(), tokens.end(), IsHigherFrequency);
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
        SortTokens(options);
        return options[0];
    }

    return nullptr;
}

bool LeavesGoodRemainder(Dictionary *dict, TaiText const &prefix, std::string_view raw_query) {
    auto size = prefix.RawSize();
    return dict->word_splitter()->CanSplit(raw_query.substr(size, raw_query.size() - size));
}

std::vector<Buffer> GetCandidatesFromStartImpl(Engine *engine, TaiToken *lgram, std::string const &raw_query) {
    auto ret = std::vector<Buffer>();

    auto segments = Segmenter::SegmentText(engine, raw_query);

    switch (segments.at(0).type) {
    case SegmentType::Punct: {
        auto query = raw_query.substr(0, segments.at(0).size);
        auto options = engine->dictionary()->SearchPunctuation(query);

        for (auto &p : options) {
            ret.push_back(Buffer(p));
            ret.back().SetConverted(true);
        }
        
        break;
    }
    case SegmentType::Splittable: {
        auto query = raw_query.substr(0, segments.at(0).size);
        auto query_lc = unicode::copy_str_tolower(query);
        auto options = engine->dictionary()->AllWordsFromStart(query_lc);

        std::sort(options.begin(), options.end(), [](TaiToken *a, TaiToken *b) {
            return unicode::letter_count(a->input) > unicode::letter_count(b->input);
        });

        auto parser = engine->syllable_parser();
        auto seen = std::unordered_set<std::string>();

        for (auto option : options) {
            if (!seen.insert(option->output).second) {
                continue;
            }

            auto tai_text = parser->AsTaiText(query, option->input);

            if (!LeavesGoodRemainder(engine->dictionary(), tai_text, query)) {
                continue;
            }

            tai_text.SetCandidate(option);
            auto elem = BufferElement(std::move(tai_text));
            elem.is_converted = true;
            ret.push_back(std::vector<BufferElement>{std::move(elem)});
        }
        break;
    }
    default:
        ret.push_back(Buffer(raw_query.substr(0, segments.at(0).size)));
        ret.back().SetConverted(true);
        break;
    }

    return ret;
}

Buffer WordsToBuffer(Engine *engine, std::vector<std::string> &words) {
    auto elems = BufferElementList();
    for (auto &word : words) {
        auto best_match = CandidateFinder::BestMatch(engine, nullptr, word);
        auto tai_text = TaiText::FromMatching(engine->syllable_parser(), word, best_match);
        tai_text.SetCandidate(best_match);
        auto elem = BufferElement(std::move(tai_text));
        elem.is_converted = true;
        elems.push_back(std::move(elem));
    }
    return Buffer(std::move(elems));
}

Buffer ContinuousBestMatchImpl(Engine *engine, TaiToken *lgram, std::string_view query) {
    auto segmentations = engine->dictionary()->Segment(query, 1);

    if (segmentations.empty()) {
        return Buffer();
    }

    return WordsToBuffer(engine, segmentations[0]);
}

std::vector<Buffer> ContinuousCandidatesImpl(Engine *engine, TaiToken *lgram, std::string const &query) {
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

std::string PunctuationBestMatchImpl(Engine *engine, std::string const &query) {
    auto puncts = engine->dictionary()->SearchPunctuation(query);
    if (!puncts.empty()) {
        return puncts[0];
    }
    return query;
}

std::vector<Buffer> MultiSegmentCandidatesImpl(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto candidates = std::vector<Buffer>{Buffer()};
    auto segments = Segmenter::SegmentText(engine, query);

    for (auto i = 0; i < segments.size(); ++i) {
        auto &seg = segments[i];
        auto seg_raw_comp = query.substr(seg.start, seg.size);

        switch (seg.type) {
        case SegmentType::Splittable:
            if (i == 0) {
                candidates = ContinuousCandidatesImpl(engine, nullptr, seg_raw_comp);
            } else {
                candidates[0].Append(ContinuousBestMatchImpl(engine, nullptr, seg_raw_comp));
            }
            break;
        case SegmentType::Hyphens:
            candidates[0].Append(std::string(seg.size, '-'));
            break;
        case SegmentType::Punct:
            if (i == 0) {
                candidates = GetCandidatesFromStartImpl(engine, nullptr, seg_raw_comp);
            } else {
                candidates[0].Append(PunctuationBestMatchImpl(engine, seg_raw_comp));
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
    return BestMatchImpl(engine, lgram, options);
}

TaiToken *CandidateFinder::BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->Autocomplete(copy_str_tolower(query));
    return BestMatchImpl(engine, lgram, options);
}

std::vector<Buffer> CandidateFinder::GetCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &query) {
    // TODO - add dictionary method for finding candidates
    return GetCandidatesFromStartImpl(engine, lgram, query);
}

std::vector<Buffer> CandidateFinder::ContinuousCandidates(Engine *engine, TaiToken *lgram, std::string const &query) {
    return ContinuousCandidatesImpl(engine, lgram, query);
}

std::vector<Buffer> CandidateFinder::MultiSegmentCandidates(Engine *engine, TaiToken *lgram, std::string const &query) {
    return MultiSegmentCandidatesImpl(engine, lgram, query);
}

Buffer CandidateFinder::ContinuousBestMatch(Engine *engine, TaiToken *lgram, std::string_view query) {
    return ContinuousBestMatchImpl(engine, lgram, query);
}

bool CandidateFinder::HasExactMatch(Engine *engine, std::string_view query) {
    return engine->dictionary()->word_splitter()->CanSplit(query);
}

} // namespace khiin::engine

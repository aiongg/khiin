#include "CandidateFinder.h"

#include "Database.h"
#include "Dictionary.h"
#include "Engine.h"
#include "Splitter.h"
#include "SyllableParser.h"
#include "Trie.h"

namespace khiin::engine {

namespace {

inline bool IsHigherFrequency(TaiToken *left, TaiToken *right) {
    return left->input_id == right->input_id ? left->weight > right->weight : left->input_id < right->input_id;
}

inline void SortTokens(std::vector<TaiToken *> &tokens) {
    std::sort(tokens.begin(), tokens.end(), IsHigherFrequency);
}

std::string RemoveRawFromStart(TaiText const &text, std::string const &raw_query) {
    auto ret = std::string();
    auto prefix = text.raw();
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

bool LeavesGoodRemainder(Dictionary *dict, TaiText const& prefix, std::string_view raw_query) {
    auto size = prefix.RawSize();
    return dict->word_splitter()->CanSplit(raw_query.substr(size, raw_query.size() - size));
}

std::vector<Buffer> GetCandidatesFromStartImpl(Engine *engine, TaiToken *lgram, std::string const &raw_query) {
    auto options = engine->dictionary()->AllWordsFromStart(raw_query);

    std::sort(options.begin(), options.end(), [](TaiToken *a, TaiToken *b) {
        return unicode::letter_count(a->input) > unicode::letter_count(b->input);
    });

    auto ret = std::vector<Buffer>();
    auto parser = engine->syllable_parser();
    auto seen = std::unordered_set<std::string>();

    for (auto option : options) {
        if (!seen.insert(option->output).second) {
            continue;
        }

        auto tai_text = parser->AsTaiText(raw_query, option->input);

        if (!LeavesGoodRemainder(engine->dictionary(), tai_text, raw_query)) {
            continue;
        }

        tai_text.SetCandidate(option);
        auto elem = BufferElement(std::move(tai_text));
        elem.is_converted = true;
        ret.push_back(std::vector<BufferElement>{std::move(elem)});
    }

    return ret;
}

std::vector<Buffer> ContinuousCandidatesImpl(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto all_cands = std::vector<Buffer>();
    auto segmentations = engine->dictionary()->Segment(query);
    auto seen = std::unordered_set<std::string>();
    auto ret = std::vector<Buffer>();

    for (auto &segmentation : segmentations) {
        auto elems = BufferElementList();
        for (auto &word : segmentation) {
            auto best_match = CandidateFinder::BestMatch(engine, nullptr, word);
            auto elem = BufferElement(TaiText::FromMatching(engine->syllable_parser(), word, best_match));
            elem.is_converted = true;
            elems.push_back(std::move(elem));
        }
        auto buf = Buffer(std::move(elems));
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

} // namespace

TaiToken *CandidateFinder::BestMatch(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->WordSearch(query);
    return BestMatchImpl(engine, lgram, options);
}

TaiToken *CandidateFinder::BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query) {
    auto options = engine->dictionary()->Autocomplete(query);
    return BestMatchImpl(engine, lgram, options);
}

std::vector<Buffer> CandidateFinder::GetCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &query) {
    // TODO - add dictionary method for finding candidates
    return GetCandidatesFromStartImpl(engine, lgram, query);
}

std::vector<Buffer> CandidateFinder::ContinuousCandidates(Engine *engine, TaiToken *lgram, std::string const &query) {
    return ContinuousCandidatesImpl(engine, lgram, query);
}

} // namespace khiin::engine

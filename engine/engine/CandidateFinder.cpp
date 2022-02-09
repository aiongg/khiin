#include "CandidateFinder.h"

#include "BufferElement.h"
#include "Database.h"
#include "Dictionary.h"
#include "Engine.h"
#include "SyllableParser.h"

namespace khiin::engine {

namespace {

TaiToken *BestMatchUnigram(Engine *engine, std::vector<TaiToken *> const &options) {
    return engine->database()->HighestUnigramCount(options);
}

TaiToken *BestMatchBigram(Engine *engine, TaiToken *lgram, std::vector<TaiToken *> const &options) {
    assert(lgram);
    return engine->database()->HighestBigramCount(lgram->output, options);
}

TaiToken *BestMatchImpl(Engine *engine, TaiToken *lgram, std::vector<TaiToken *> const &options) {
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
        return options[0];
    }

    return nullptr;
}

std::vector<BufferElementList> GetCandidatesFromStartImpl(Engine *engine, TaiToken *lgram,
                                                                   std::string const &raw_query,
                                                                   std::vector<TaiToken *> &options) {
    std::sort(options.begin(), options.end(), [](TaiToken *a, TaiToken *b) {
        return unicode::letter_count(a->input) > unicode::letter_count(b->input);
    });

    auto ret = std::vector<BufferElementList>();
    auto parser = engine->syllable_parser();

    for (auto option : options) {
        auto tai_text = parser->AsTaiText(raw_query, option->input);
        tai_text.SetCandidate(option);
        auto elem = BufferElement(std::move(tai_text));
        elem.is_converted = true;
        ret.push_back(std::vector<BufferElement>{std::move(elem)});
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

std::vector<BufferElementList> CandidateFinder::GetCandidatesFromStart(Engine *engine, TaiToken *lgram,
                                                                                std::string const &query) {
    // TODO - add dictionary method for finding candidates
    auto options = engine->dictionary()->AllWordsFromStart(query);
    return GetCandidatesFromStartImpl(engine, lgram, query, options);
}

} // namespace khiin::engine

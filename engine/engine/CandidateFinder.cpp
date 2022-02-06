#include "CandidateFinder.h"

#include "Database.h"
#include "Dictionary.h"
#include "Engine.h"

namespace khiin::engine {

namespace {

DictionaryRow *BestMatchUnigram(Engine *engine, std::vector<DictionaryRow *> const &options) {
    return engine->database()->HighestUnigramCount(options);
}

DictionaryRow *BestMatchBigram(Engine *engine, DictionaryRow *lgram, std::vector<DictionaryRow *> const &options) {
    assert(lgram);
    return engine->database()->HighestBigramCount(lgram->output, options);
}

DictionaryRow *BestMatchImpl(Engine *engine, DictionaryRow *lgram, std::vector<DictionaryRow *> const &options) {
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

} // namespace

DictionaryRow *CandidateFinder::BestMatch(Engine *engine, DictionaryRow *lgram, std::string const &query) {
    auto options = engine->dictionary()->WordSearch(query);
    return BestMatchImpl(engine, lgram, options);
}

DictionaryRow *CandidateFinder::BestAutocomplete(Engine *engine, DictionaryRow *lgram, std::string const &query) {
    auto options = engine->dictionary()->Autocomplete(query);
    return BestMatchImpl(engine, lgram, options);
}

} // namespace khiin::engine

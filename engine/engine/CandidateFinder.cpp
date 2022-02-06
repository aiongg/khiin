#include "CandidateFinder.h"

#include "Database.h"
#include "Dictionary.h"
#include "Engine.h"

namespace khiin::engine {

namespace {

DictionaryRow *BestMatchUnigram(Engine *engine, std::string const &query) {
    auto words = engine->dictionary()->WordSearch(query);
    return engine->database()->HighestUnigramCount(words);
}

DictionaryRow *BestMatchBigram(Engine *engine, DictionaryRow *lgram, std::string const &query) {
    assert(lgram);

    auto words = engine->dictionary()->WordSearch(query);
    return engine->database()->HighestBigramCount(lgram->output, words);
}

} // namespace

DictionaryRow *CandidateFinder::BestMatch(Engine *engine, DictionaryRow *lgram, std::string const &query) {
    if (lgram) {
        auto ret = BestMatchBigram(engine, lgram, query);
        if (ret != nullptr) {
            return ret;
        }
    }
    
    return BestMatchUnigram(engine, query);
}

} // namespace khiin::engine

#pragma once

#include <string>

#include "Models.h"

namespace khiin::engine {

class Engine;

class CandidateFinder {
  public:
    static DictionaryRow *BestMatch(Engine *engine, DictionaryRow *lgram, std::string const &query);
    static DictionaryRow *BestAutocomplete(Engine *engine, DictionaryRow *lgram, std::string const &query);
};

} // namespace khiin::engine

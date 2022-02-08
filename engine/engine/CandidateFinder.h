#pragma once

#include <string>
#include <vector>

#include "Models.h"
#include "BufferElement.h"

namespace khiin::engine {

class Engine;

class CandidateFinder {
  public:
    static TaiToken *BestMatch(Engine *engine, TaiToken *lgram, std::string const &query);
    static TaiToken *BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query);
    static std::vector<BufferElementList> GetCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &query);
};

} // namespace khiin::engine

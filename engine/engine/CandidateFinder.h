#pragma once

#include <string>
#include <vector>

#include "Buffer.h"
#include "Models.h"

namespace khiin::engine {

class Engine;

class CandidateFinder {
  public:
    static TaiToken *BestMatch(Engine *engine, TaiToken *lgram, std::string const &query);
    static TaiToken *BestAutocomplete(Engine *engine, TaiToken *lgram, std::string const &query);
    static std::vector<Buffer> GetCandidatesFromStart(Engine *engine, TaiToken *lgram, std::string const &query);
    static std::vector<Buffer> ContinuousCandidates(Engine *engine, TaiToken *lgram, std::string const &query);
    static bool HasExactMatch(Engine *engine, std::string_view query);
    static Buffer ContinuousBestMatch(Engine *engine, TaiToken *lgram, std::string_view query);
    static std::vector<Buffer> MultiSegmentCandidates(Engine *engine, TaiToken *lgram, std::string const &query);
};

} // namespace khiin::engine

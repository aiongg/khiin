#pragma once

#include <string>
#include <vector>

#include "Buffer.h"
#include "data/Models.h"

namespace khiin::engine {

class Engine;

class CandidateFinder {
  public:
    static std::vector<Buffer> MultiMatch(Engine *engine, TaiToken *lgram, std::string const &query);
    static Buffer ContinuousSingleMatch(Engine *engine, TaiToken *lgram, std::string const &query);
    static std::vector<Buffer> ContinuousMultiMatch(Engine *engine, TaiToken *lgram, std::string const &query);
    static bool HasExactMatch(Engine *engine, std::string_view query);
};

} // namespace khiin::engine

#pragma once

#include <string>
#include <vector>

#include "Buffer.h"
#include "Models.h"

namespace khiin::engine {

class Engine;

class CandidateFinder {
  public:

    static std::vector<Buffer> WordsByLength(Engine *engine, TaiToken *lgram, std::string const &query);
    static std::vector<Buffer> WordsByWeight(Engine *engine, TaiToken *lgram, std::string const &query);
    static Buffer ContinuousBestMatch(Engine *engine, TaiToken *lgram, std::string_view query);
    static std::vector<Buffer> ContinuousMultiMatch(Engine *engine, TaiToken *lgram, std::string const &query);
    static bool HasExactMatch(Engine *engine, std::string_view query);
};

} // namespace khiin::engine

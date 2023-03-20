#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Buffer.h"
#include "data/Models.h"

namespace khiin::engine {

class Engine;

class CandidateFinder {
   public:
    static std::vector<Buffer> MultiMatch(
        Engine* engine,
        std::optional<TaiToken> const& lgram,
        std::string const& query);

    static Buffer ContinuousSingleMatch(
        Engine* engine,
        std::optional<TaiToken> const& lgram,
        std::string const& query);

    static std::vector<Buffer> ContinuousMultiMatch(
        Engine* engine,
        std::optional<TaiToken> const& lgram,
        std::string const& query);

    static bool HasExactMatch(Engine* engine, std::string_view query);
};

}  // namespace khiin::engine

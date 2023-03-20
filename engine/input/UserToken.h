#pragma once

#include <string>

namespace khiin::engine {

struct TaiToken;

struct UserToken {
    std::string input;
    std::string output;

    std::string Output() const;
    size_t InputSize() const;
    size_t OutputSize() const;
};

} // namespace khiin::engine

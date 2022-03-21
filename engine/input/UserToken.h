#pragma once

#include <string>

namespace khiin::engine {

struct TaiToken;

struct UserToken {
    UserToken(std::string const &input, TaiToken *candidate);

    std::string Input() const;
    std::string Output() const;
    size_t InputSize() const;
    size_t OutputSize() const;
    TaiToken *candidate() const;

  private:
    std::string m_input;
    TaiToken *m_candidate = nullptr;
};

} // namespace khiin::engine

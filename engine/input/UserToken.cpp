#include "UserToken.h"

#include "data/Models.h"
#include "utils/unicode.h"

namespace khiin::engine {
using namespace khiin::unicode;

UserToken::UserToken(std::string const &input, TaiToken *candidate) : m_input(input), m_candidate(candidate) {}

std::string UserToken::Input() const {
    return m_input;
}

std::string UserToken::Output() const {
    if (m_candidate != nullptr) {
        return m_candidate->output;
    }

    return Input();
}

size_t UserToken::InputSize() const {
    return u8_size(Input());
}

size_t UserToken::OutputSize() const {
    return u8_size(Output());
}

TaiToken *UserToken::candidate() const {
    return m_candidate;
}

} // namespace khiin::engine

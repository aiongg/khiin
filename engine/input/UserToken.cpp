#include "UserToken.h"

#include "data/Models.h"
#include "utils/unicode.h"

namespace khiin::engine {
using namespace khiin::unicode;

std::string UserToken::Output() const {
    return output.empty() ? input : output;
}

size_t UserToken::InputSize() const {
    return u8_size(input);
}

size_t UserToken::OutputSize() const {
    return u8_size(Output());
}

} // namespace khiin::engine

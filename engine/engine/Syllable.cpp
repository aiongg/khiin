#include "Syllable.h"

#include <assert.h>

#include <utf8cpp/utf8.h>

namespace khiin::engine {

bool Syllable::operator==(Syllable const &rhs) const {
    return this->raw_body == rhs.raw_body && this->tone == rhs.tone && this->khin_pos == rhs.khin_pos;
}

} // namespace khiin::engine

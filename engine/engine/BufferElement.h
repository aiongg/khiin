#pragma once

#include <variant>

#include "BufferSegment.h"

namespace khiin::engine {

class Punctuation {};
using Plaintext = std::string;

class BufferElement {
  public:
    utf8_size_t Size();

  private:
    std::variant<Plaintext, BufferSegment> m_element;
};

} // namespace khiin::engine

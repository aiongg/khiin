#pragma once

#include <string>
#include <variant>

#include "BufferSegment.h"

namespace khiin::engine {

class Punctuation {};
using Plaintext = std::string;

class BufferElement {
  public:
    BufferElement(BufferSegment const &elem);
    BufferElement(Plaintext const &elem);
    utf8_size_t Size();
    std::string Raw();
    void RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret);

  private:
    std::variant<Plaintext, BufferSegment> m_element;
};

} // namespace khiin::engine

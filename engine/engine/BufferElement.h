#pragma once

#include <string>
#include <variant>

#include "TaiText.h"

namespace khiin::engine {

class SyllableParser;
class Punctuation {};
using Plaintext = std::string;

class BufferElement {
  public:
    BufferElement(TaiText const &elem);
    BufferElement(Plaintext const &elem);
    utf8_size_t size();
    std::string raw();
    void RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret);

    std::string composed();
    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret);

  private:
    std::variant<Plaintext, TaiText> m_element;
};

} // namespace khiin::engine

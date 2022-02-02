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
    BufferElement(Spacer elem);
    utf8_size_t size();
    std::string raw();

    std::string composed();
    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret);
    size_t ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret);

  private:
    std::variant<Plaintext, TaiText, Spacer> m_element;
};

} // namespace khiin::engine

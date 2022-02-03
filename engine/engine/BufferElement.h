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
    BufferElement();
    BufferElement(TaiText const &elem);
    BufferElement(Plaintext const &elem);
    BufferElement(Spacer elem);
    utf8_size_t size() const;
    std::string raw() const;
    std::string composed() const;
    std::string converted() const;

    void Erase(SyllableParser *parser, utf8_size_t index);

    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const;
    size_t ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const;
    bool IsVirtualSpace(utf8_size_t index) const;

  private:
    std::variant<std::monostate, Plaintext, TaiText, Spacer> m_element;
};

} // namespace khiin::engine

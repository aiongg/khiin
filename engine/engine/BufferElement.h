#pragma once

#include <string>
#include <variant>

#include "TaiText.h"

namespace khiin::engine {

class SyllableParser;
struct TaiToken;
class Punctuation {};
using Plaintext = std::string;

class BufferElement {
  public:
    BufferElement();
    BufferElement(TaiText const &elem);
    BufferElement(Plaintext const &elem);
    BufferElement(Spacer elem);

    static std::string raw(std::vector<BufferElement> const &elements);

    void Replace(TaiText const &elem);
    void Replace(Plaintext const &elem);
    void Replace(Spacer elem);

    utf8_size_t size() const;
    std::string raw() const;
    std::string composed() const;
    std::string converted() const;
    TaiToken *candidate() const;

    void Erase(SyllableParser *parser, utf8_size_t index);
    bool SetKhin(SyllableParser *parser, KhinKeyPosition khin_pos, char khin_key);

    utf8_size_t RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const;
    size_t ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const;

    bool IsVirtualSpace() const;
    bool IsVirtualSpace(utf8_size_t index) const;

    bool is_converted = false;

  private:
    std::variant<std::monostate, Plaintext, TaiText, Spacer> m_element;
};

using BufferElementList = std::vector<BufferElement>;

} // namespace khiin::engine

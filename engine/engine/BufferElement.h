#pragma once

#include <string>
#include <variant>

#include "TaiText.h"

namespace khiin::engine {

class SyllableParser;
struct TaiToken;

class BufferElement {
  public:
    static bool ConvertedEq(BufferElement const &lhs, BufferElement const &rhs);

    BufferElement();
    BufferElement(TaiText const &elem);
    BufferElement(std::string const &elem);
    BufferElement(VirtualSpace elem);

    bool operator==(BufferElement const &rhs) const;

    void Replace(TaiText const &elem);
    void Replace(std::string const &elem);
    void Replace(VirtualSpace elem);

    utf8_size_t size() const;
    utf8_size_t RawSize() const;

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
    std::variant<std::monostate, std::string, TaiText, VirtualSpace> m_element;
};

using BufferElementList = std::vector<BufferElement>;

} // namespace khiin::engine

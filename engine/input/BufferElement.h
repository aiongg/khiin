#pragma once

#include <string>
#include <variant>
#include <vector>

#include "TaiText.h"
#include "UserToken.h"

namespace khiin::engine {
using utf8_size_t = size_t;

class BufferElement {
  public:
    static bool ConvertedEq(BufferElement const &lhs, BufferElement const &rhs);
    static BufferElement Build(SyllableParser *parser, std::string const &input, TaiToken *match, bool set_candidate,
                               bool set_converted);

    BufferElement();
    BufferElement(TaiText const &elem);
    BufferElement(TaiText &&elem);
    BufferElement(Punctuation const &elem);
    BufferElement(Punctuation &&elem);
    BufferElement(std::string const &elem);
    BufferElement(std::string &&elem);
    BufferElement(VirtualSpace elem);
    BufferElement(UserToken &&elem);

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

    void Erase(utf8_size_t index);
    bool SetKhin(KhinKeyPosition khin_pos, char khin_key);

    utf8_size_t RawToComposedCaret(size_t raw_caret) const;
    size_t ComposedToRawCaret(utf8_size_t caret) const;

    bool IsVirtualSpace() const;
    bool IsVirtualSpace(utf8_size_t index) const;
    bool IsTaiText() const noexcept;
    bool IsUserToken() const noexcept;
    bool IsConverted() const noexcept;
    bool IsSelected() const noexcept;
    void SetConverted(bool converted) noexcept;
    void SetSelected(bool selected) noexcept;

  private:
    bool is_converted = false;
    bool is_selected = false;
    std::variant<std::monostate, std::string, TaiText, Punctuation, VirtualSpace, UserToken> m_element;
};

using BufferElementList = std::vector<BufferElement>;

} // namespace khiin::engine

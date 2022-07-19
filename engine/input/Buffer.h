#pragma once

#include "BufferElement.h"

namespace khiin::engine {

class SyllableParser;

// A simple wrapper class around a vector of BufferElements
// with some utility functions for manipulating the vector.
// The underlying vector can be obtained and manipulated directly
// with ::get() where required.
class Buffer {
  public:
    using iterator = BufferElementList::iterator;
    using const_iterator = BufferElementList::const_iterator;

    // Returns size of composed or converted buffer in unicode codepoints
    static utf8_size_t TextSize(const_iterator const &from, const_iterator const &to);

    // Returns size of the raw buffer in unicode codepoints
    static utf8_size_t RawTextSize(const_iterator const &from, const_iterator const &to);

    // Returns the raw text between |from| and |to|
    static std::string RawText(const_iterator const &from, const_iterator const &to);

    // Adds virtual spacer elements where required
    // (e.g., between loji and hanji).
    //static void AdjustVirtualSpacing(BufferElementList &elements);

    Buffer() = default;
    explicit Buffer(BufferElementList &&elements);
    explicit Buffer(BufferElement &&element);

    iterator Begin() noexcept;
    iterator End() noexcept;
    const_iterator CBegin() const;
    const_iterator CEnd() const;
    BufferElement &At(size_t index);
    BufferElement &Back() noexcept;
    size_t Size() const;
    iterator Erase(iterator it);

    // Returns iterator pointing to the element at the caret, using the
    // visible caret position
    iterator IterCaret(size_t caret);
    const_iterator CIterCaret(size_t caret) const;

    // Returns iterator pointing to the element at the caret, using the
    // underlying raw caret position
    iterator IterRawCaret(size_t raw_caret);
    const_iterator CIterRawCaret(size_t caret) const;

    void Clear();
    bool Empty() const;

    // Input |caret| is the visible displayed caret, in unicode code points. Returns
    // the corresponding raw caret.
    size_t RawCaretFrom(utf8_size_t caret) const;

    // Input |raw_caret| is the raw caret position, in unicode code points. Returns
    // the corresponding display caret position.
    utf8_size_t CaretFrom(utf8_size_t raw_caret) const;

    std::string RawText() const;
    utf8_size_t RawTextSize() const;
    std::string Text() const;
    utf8_size_t TextSize() const;

    std::string RawTextFrom(size_t element_index) const;

    bool AllComposing() const;

    // Returns true if there are any non-converted elements in the compostion
    bool HasComposing() const;

    // Moves converted sections before and after composition
    // into separate holders |pre| and |post|. Buffer must be re-joined later.
    void IsolateComposing(Buffer &pre, Buffer &post);

    // Only call this when the whole buffer is converted. Buffer will be split
    // at the caret if the caret element is Hanji, or keep the caret element
    // in place if it is Lomaji. Buffer must be re-joined later.
    void SplitForComposition(utf8_size_t caret, Buffer &pre, Buffer &post);

    // Move all elements up to |index| into |pre|, and all elements
    // after |index| into |post|. Either |pre| or |post| may be nullptr.
    void SplitAtElement(size_t index, Buffer *pre, Buffer *post);

    void Join(Buffer *pre, Buffer *post);
    void RemoveVirtualSpacing();
    void AdjustVirtualSpacing();
    void StripVirtualSpacing();

    // Creates the appropriate variant internally
    void Append(SyllableParser *parser, std::string const &input, TaiToken *match = nullptr, bool set_candidate = false,
                bool set_converted = false);
    void Append(Buffer &rhs);
    void Append(Buffer &&rhs);
    void Append(BufferElement &&rhs);
    void Append(std::string &&rhs);
    void Append(TaiText &&rhs);
    void Append(Punctuation &&rhs);

    // Replace element at |index| in this Buffer with all elements from |replace|
    iterator Replace(iterator first, iterator last, Buffer &other);

    void SetConverted(bool converted);
    void SetSelected(bool selected);

    BufferElementList &get();

  private:
    BufferElementList m_elements;
};

} // namespace khiin::engine

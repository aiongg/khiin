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
    static utf8_size_t Size(iterator const &from, iterator const &to);

    // Returns size of the raw buffer in unicode codepoints
    static utf8_size_t RawSize(iterator const &from, iterator const &to);

    // Returns the raw text between |from| and |to|
    static std::string RawText(iterator const &from, iterator const &to);

    // Adds virtual spacer elements where required
    // (e.g., between loji and hanji).
    static void AdjustVirtualSpacing(BufferElementList &elements);

    Buffer() = default;
    Buffer(BufferElementList &&m_elements);

    iterator Begin();
    iterator End();

    // Returns iterator pointing to the element at the caret, using the
    // visible caret position
    iterator At(utf8_size_t caret);

    // Returns iterator pointing to the element at the caret, using the
    // underlying raw caret position
    iterator AtRaw(utf8_size_t raw_caret);
    const_iterator CBegin();
    const_iterator CEnd();

    void Clear();
    bool Empty();
    utf8_size_t Size();
    size_t RawSize();

    // Input |caret| is the visible displayed caret, in unicode code points. Returns
    // the corresponding raw caret.
    size_t RawCaretFrom(utf8_size_t caret, SyllableParser *parser);

    // Input |raw_caret| is the raw caret position, in unicode code points. Returns
    // the corresponding display caret position.
    utf8_size_t CaretFrom(utf8_size_t raw_caret, SyllableParser *parser);

    std::string RawText();
    std::string Text();

    // Returns true if there are any non-converted elements in the compostion
    bool HasComposing();

    // Moves converted sections before and after composition
    // into separate holders |pre| and |post|. Buffer must be re-joined later.
    void IsolateComposing(Buffer &pre, Buffer &post);

    // Only call this when the whole buffer is converted. Buffer will be split
    // at the caret if the caret element is Hanji, or keep the caret element
    // in place if it is Lomaji. Buffer must be re-joined later.
    void SplitForComposition(utf8_size_t caret, Buffer &pre, Buffer &post);

    utf8_size_t Join(utf8_size_t raw_caret, Buffer &pre, Buffer &post);
    void AdjustVirtualSpacing();

    BufferElementList &get();

  private:
    BufferElementList m_elements;
};

} // namespace khiin::engine
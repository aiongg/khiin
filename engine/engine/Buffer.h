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
    static utf8_size_t TextSize(iterator const &from, iterator const &to);

    // Returns size of the raw buffer in unicode codepoints
    static utf8_size_t RawTextSize(iterator const &from, iterator const &to);

    // Returns the raw text between |from| and |to|
    static std::string RawText(iterator const &from, iterator const &to);

    // Adds virtual spacer elements where required
    // (e.g., between loji and hanji).
    static void AdjustVirtualSpacing(BufferElementList &elements);

    Buffer() = default;
    Buffer(BufferElementList &&elements);
    Buffer(BufferElement &&element);

    iterator Begin();
    iterator End();
    const_iterator CBegin();
    const_iterator CEnd();
    BufferElement &At(size_t index);
    BufferElement &Back();
    size_t Size();
    iterator Erase(iterator it);

    // Returns iterator pointing to the element at the caret, using the
    // visible caret position
    iterator IterCaret(utf8_size_t caret);

    // Returns iterator pointing to the element at the caret, using the
    // underlying raw caret position
    iterator IterRawCaret(utf8_size_t raw_caret);

    void Clear();
    bool Empty();

    // Input |caret| is the visible displayed caret, in unicode code points. Returns
    // the corresponding raw caret.
    size_t RawCaretFrom(utf8_size_t caret);

    // Input |raw_caret| is the raw caret position, in unicode code points. Returns
    // the corresponding display caret position.
    utf8_size_t CaretFrom(utf8_size_t raw_caret);

    std::string RawText();
    utf8_size_t RawTextSize();
    std::string Text();
    utf8_size_t TextSize();

    // Returns true if there are any non-converted elements in the compostion
    bool HasComposing();

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
    void AdjustVirtualSpacing();
    void StripVirtualSpacing();

    // Append all elements of |rhs| to this Buffer
    void Append(Buffer &rhs);

    // Append a plain text string as a new buffer element
    void Append(std::string &&str);

    // Append a TaiText as a new buffer element
    void Append(TaiText &&taitext);

    // Append a Punctuation as a new buffer element
    void Append(Punctuation &&punct);

    // Replace element at |index| in this Buffer with all elements from |replace|
    void Replace(iterator first, iterator last, Buffer &other);

    void SetConverted(bool converted);

    BufferElementList &get();

  private:
    BufferElementList m_elements;
};

} // namespace khiin::engine

#include "Buffer.h"

#include <assert.h>

#include "SyllableParser.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {
using namespace unicode;

void RemoveVirtualSpaces(BufferElementList &elements) {
    auto it = std::remove_if(elements.begin(), elements.end(), [](BufferElement const &el) {
        return el.IsVirtualSpace();
    });
    elements.erase(it, elements.end());
}

bool NeedsVirtualSpace(std::string const &lhs, std::string const &rhs) {
    auto left = end_glyph_type(lhs);
    auto right = start_glyph_type(rhs);

    if (left == GlyphCategory::Alnum && right == GlyphCategory::Alnum ||
        left == GlyphCategory::Alnum && right == GlyphCategory::Khin ||
        left == GlyphCategory::Alnum && right == GlyphCategory::Hanji ||
        left == GlyphCategory::Hanji && right == GlyphCategory::Alnum) {
        return true;
    }
    return false;
}

} // namespace

using iterator = BufferElementList::iterator;
using const_iterator = BufferElementList::const_iterator;

utf8_size_t Buffer::TextSize(iterator const &from, iterator const &to) {
    auto it = from;
    utf8_size_t size = 0;

    for (; it != to; ++it) {
        size += it->size();
    }

    return size;
}

utf8_size_t Buffer::RawTextSize(iterator const &from, iterator const &to) {
    auto it = from;
    utf8_size_t size = 0;

    for (; it != to; ++it) {
        size += it->RawSize();
    }

    return size;
}

std::string Buffer::RawText(iterator const &from, iterator const &to) {
    auto ret = std::string();
    auto it = from;
    for (; it != to; ++it) {
        ret.append(it->raw());
    }
    return ret;
}

void Buffer::AdjustVirtualSpacing(BufferElementList &elements) {
    if (elements.empty()) {
        return;
    }

    RemoveVirtualSpaces(elements);

    for (auto i = elements.size() - 1; i != 0; --i) {
        std::string lhs =
            elements.at(i - 1).is_converted ? elements.at(i - 1).converted() : elements.at(i - 1).composed();
        std::string rhs = elements.at(i).is_converted ? elements.at(i).converted() : elements.at(i).composed();

        if (NeedsVirtualSpace(lhs, rhs)) {
            auto tmp = BufferElement(VirtualSpace());
            if (elements.at(i - 1).is_converted && elements.at(i).is_converted) {
                tmp.is_converted = true;
            }
            elements.insert(elements.begin() + i, std::move(tmp));
        }
    }
}

// Not currently used
void Buffer::StripVirtualSpacing() {
    if (m_elements.empty()) {
        return;
    }

    auto begin = Begin();
    auto it = Begin();
    auto end = End();
    while (it != end && it->IsVirtualSpace()) {
        ++it;
    }
    if (std::distance(begin, it) > 0) {
        begin = m_elements.erase(begin, it);
    }

    if (m_elements.empty()) {
        return;
    }

    end = End();
    it = end;
    while (it != begin && (it - 1)->IsVirtualSpace()) {
        --it;
    }
    if (std::distance(it, end) > 0) {
        m_elements.erase(it, end);
    }
}

Buffer::Buffer(BufferElementList &&elements) : m_elements(elements) {}

Buffer::Buffer(BufferElement &&element) {
    m_elements.push_back(std::move(element));
}

iterator Buffer::Begin() {
    return m_elements.begin();
}

iterator Buffer::End() {
    return m_elements.end();
}

BufferElement &Buffer::At(size_t index) {
    return m_elements.at(index);
}

BufferElement &Buffer::Back() {
    return m_elements.back();
}

size_t Buffer::Size() {
    return m_elements.size();
}

void Buffer::Append(Buffer &rhs) {
    m_elements.insert(m_elements.end(), rhs.Begin(), rhs.End());
}

void Buffer::Append(std::string &&elem) {
    m_elements.push_back(BufferElement(std::move(elem)));
}

void Buffer::Append(TaiText &&elem) {
    m_elements.push_back(BufferElement(std::move(elem)));
}

void Buffer::Append(Punctuation &&elem) {
    m_elements.push_back(BufferElement(std::move(elem)));
}

iterator Buffer::Erase(iterator it) {
    return m_elements.erase(it);
}

// Returns iterator pointing to the element at visible caret position
iterator Buffer::IterCaret(utf8_size_t caret) {
    auto rem = caret;
    auto it = Begin();
    auto end = End();
    for (; it != end; ++it) {
        if (auto size = it->size(); rem > size) {
            rem -= size;
        } else {
            break;
        }
    }
    return it;
}

// Returns iterator pointing to the element at raw caret position
iterator Buffer::IterRawCaret(utf8_size_t raw_caret) {
    auto rem = raw_caret;
    auto it = Begin();
    auto end = End();
    for (; it != end; ++it) {
        if (auto size = it->RawSize(); rem > size) {
            rem -= size;
        } else {
            break;
        }
    }
    return it;
}

const_iterator Buffer::CBegin() {
    return m_elements.cbegin();
}

const_iterator Buffer::CEnd() {
    return m_elements.cend();
}

void Buffer::Clear() {
    m_elements.clear();
}

bool Buffer::Empty() {
    return m_elements.empty();
}

utf8_size_t Buffer::TextSize() {
    return TextSize(Begin(), End());
}

size_t Buffer::RawTextSize() {
    return RawTextSize(Begin(), End());
}

// Input |caret| is the visible displayed caret, in unicode code points. Returns
// the corresponding raw caret.
size_t Buffer::RawCaretFrom(utf8_size_t caret) {
    if (Empty()) {
        return 0;
    }

    auto begin = Begin();
    auto it = IterCaret(caret);

    if (it == End()) {
        return RawTextSize();
    }

    assert(it != End());
    auto remainder = caret - TextSize(begin, it);
    auto raw_remainder = it->ComposedToRawCaret(remainder);
    return RawTextSize(begin, it) + raw_remainder;
}

utf8_size_t Buffer::CaretFrom(utf8_size_t raw_caret) {
    if (Empty()) {
        return 0;
    }
    auto begin = Begin();
    auto it = IterRawCaret(raw_caret);

    if (it == End()) {
        return TextSize();
    }

    assert(it != End());
    auto raw_remainder = raw_caret - RawTextSize(begin, it);
    auto remainder = it->RawToComposedCaret(raw_remainder);
    return TextSize(begin, it) + remainder;
}

std::string Buffer::RawText() {
    return RawText(Begin(), End());
}

std::string Buffer::Text() {
    auto ret = std::string();
    for (auto &elem : m_elements) {
        if (elem.is_converted) {
            ret.append(elem.converted());
        } else {
            ret.append(elem.composed());
        }
    }
    return ret;
}

bool Buffer::HasComposing() {
    for (auto &el : m_elements) {
        if (!el.is_converted) {
            return true;
        }
    }

    return false;
}

// Moves converted sections before and after composition
// into separate holders |pre| and |post|. Buffer must be re-joined later.
void Buffer::IsolateComposing(Buffer &pre, Buffer &post) {
    auto begin = Begin();
    auto it = Begin();
    auto end = End();
    while (it != end && it->is_converted) {
        ++it;
    }

    if (it != begin) {
        pre.get().insert(pre.Begin(), begin, it);
        it = m_elements.erase(begin, it);
        end = End();
    }

    while (it != end && !it->is_converted) {
        ++it;
    }

    if (it != end) {
        post.get().insert(post.Begin(), it, end);
        m_elements.erase(it, end);
    }
}

// Only call this when the whole buffer is converted. Buffer will be split
// at the caret if the caret element is Hanji, or keep the caret element
// in place if it is Lomaji. Buffer must be re-joined later.
void Buffer::SplitForComposition(utf8_size_t caret, Buffer &pre, Buffer &post) {
    auto begin = Begin();
    auto elem = IterCaret(caret);

    if (begin != elem) {
        pre.get().insert(pre.Begin(), begin, elem);
        elem = m_elements.erase(begin, elem);
    }

    auto end = End();
    if (elem != end && elem != end - 1) {
        ++elem;
        post.get().insert(post.Begin(), elem, end);
        m_elements.erase(elem, end);
    }

    // Only one element remaining: if Hanji we split it and
    // start a new composition buffer, if Lomaji we keep it as-is
    if (auto converted = Begin()->converted(); unicode::contains_hanji(converted)) {
        auto remainder = caret - pre.TextSize();

        auto it = converted.begin();
        utf8::unchecked::advance(it, remainder);
        if (it != converted.begin()) {
            auto tmp = BufferElement(std::string(converted.begin(), it));
            tmp.is_converted = true;
            tmp.is_selected = true;
            pre.get().push_back(std::move(tmp));
        }
        if (it != converted.end()) {
            auto tmp = BufferElement(std::string(it, converted.end()));
            tmp.is_converted = true;
            tmp.is_selected = true;
            post.get().insert(post.Begin(), std::move(tmp));
        }
        Clear();
    }
}

void Buffer::SplitAtElement(size_t index, Buffer *pre, Buffer *post) {
    assert(index < m_elements.size());

    if (index == 0) {
        return;
    }

    auto begin = m_elements.begin();
    auto it = begin + index;

    if (pre) {
        pre->get().insert(pre->Begin(), begin, it);
        it = m_elements.erase(begin, it);
    }

    if (post && std::distance(it, m_elements.end()) > 1) {
        ++it;
        post->get().insert(post->Begin(), it, m_elements.end());
        m_elements.erase(it, m_elements.end());
    }
}

void Buffer::Join(Buffer *pre, Buffer *post) {
    if (pre && !pre->Empty()) {
        m_elements.insert(Begin(), pre->Begin(), pre->End());
        pre->Clear();
    }

    if (post && !post->Empty()) {
        m_elements.insert(End(), post->Begin(), post->End());
        post->Clear();
    }
}

iterator Buffer::Replace(iterator first, iterator last, Buffer &other) {
    auto it = m_elements.erase(first, last);
    return m_elements.insert(it, other.Begin(), other.End());
}

std::string ConvertedOrComposedText(BufferElement& element) {
    if (element.is_converted) {
        return element.converted();
    }

    return element.composed();
}

void Buffer::AdjustVirtualSpacing() {
    if (m_elements.empty()) {
        return;
    }

    RemoveVirtualSpaces(m_elements);

    for (auto i = m_elements.size() - 1; i != 0; --i) {
        auto lhs = ConvertedOrComposedText(m_elements.at(i - 1));
        auto rhs = ConvertedOrComposedText(m_elements.at(i));

        if (NeedsVirtualSpace(lhs, rhs)) {
            auto tmp = BufferElement(VirtualSpace());
            if (m_elements.at(i - 1).is_converted && m_elements.at(i).is_converted) {
                tmp.is_converted = true;
            } else if (m_elements.at(i - 1).is_selected && m_elements.at(i).is_selected) {
                tmp.is_selected = true;
            }
            m_elements.insert(m_elements.begin() + i, std::move(tmp));
        }
    }
    //caret = CaretFrom(raw_caret);
}

void Buffer::SetConverted(bool converted) {
    for (auto &elem : m_elements) {
        elem.is_converted = converted;
    }
}

BufferElementList &Buffer::get() {
    return m_elements;
}

} // namespace khiin::engine

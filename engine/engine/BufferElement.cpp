#include "BufferElement.h"

#include "SyllableParser.h"
#include "common.h"

namespace khiin::engine {
namespace {} // namespace

BufferElement::BufferElement(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

BufferElement::BufferElement(Plaintext const &elem) {
    m_element.emplace<Plaintext>(elem);
}

BufferElement::BufferElement(Spacer elem) {
    m_element.emplace<Spacer>(elem);
}

utf8_size_t BufferElement::size() {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return elem->size();
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->size();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        return utf8_size_t(1);
    } else {
        return utf8_size_t(0);
    }
}

std::string BufferElement::raw() {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return *elem;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->raw();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::Hyphen:
            return u8"-";
        case Spacer::Space:
            return u8" ";
        case Spacer::VirtualSpace:
            return u8"";
        default:
            return u8"";
        }
    } else {
        return u8"";
    }
}

utf8_size_t BufferElement::RawToComposedCaret(SyllableParser *parser, size_t raw_caret) {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return raw_caret;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->RawToComposedCaret(parser, raw_caret);
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        return utf8_size_t(1);
    } else {
        return utf8_size_t(0);
    }
}

size_t BufferElement::ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return caret;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->ComposedToRawCaret(parser, caret);
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::VirtualSpace:
            return size_t(0);
        default:
            return size_t(1);
        }
    } else {
        return size_t(0);
    }
}

std::string BufferElement::composed() {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return *elem;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->composed();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::Hyphen:
            return u8"-";
        case Spacer::Space:
            [[fallthrough]];
        case Spacer::VirtualSpace:
            return u8" ";
        default:
            return u8"";
        }
    } else {
        return u8"";
    }
}

std::string BufferElement::converted() {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return *elem;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->converted();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::Hyphen:
            return u8"-";
        case Spacer::Space:
            return " ";
        case Spacer::VirtualSpace:
            return u8"";
        default:
            return u8"";
        }
    } else {
        return u8"";
    }
}

} // namespace khiin::engine

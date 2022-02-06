#include "BufferElement.h"

#include "Models.h"
#include "SyllableParser.h"
#include "common.h"

namespace khiin::engine {
namespace {} // namespace

BufferElement::BufferElement() {}

BufferElement::BufferElement(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

BufferElement::BufferElement(Plaintext const &elem) {
    m_element.emplace<Plaintext>(elem);
}

BufferElement::BufferElement(Spacer elem) {
    m_element.emplace<Spacer>(elem);
}

std::string BufferElement::raw(std::vector<BufferElement> const &elements) {
    auto ret = std::string();
    for (auto const &elem : elements) {
        ret.append(elem.raw());
    }
    return ret;
}

void BufferElement::Replace(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

void BufferElement::Replace(Plaintext const &elem) {
    m_element.emplace<Plaintext>(elem);
}

void BufferElement::Replace(Spacer elem) {
    m_element.emplace<Spacer>(elem);
}

utf8_size_t BufferElement::size() const {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return elem->size();
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->size();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        return utf8_size_t(1);
    } else { // std::monostate
        return utf8_size_t(0);
    }
}

std::string BufferElement::raw() const {
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
        default:
            return "";
        }
    } else { // std::monostate
        return "";
    }
}

utf8_size_t BufferElement::RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return raw_caret;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->RawToComposedCaret(parser, raw_caret);
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        return utf8_size_t(1);
    }
    return 0; // std::monostate
}

size_t BufferElement::ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return caret;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->ComposedToRawCaret(parser, caret);
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::Space:
            [[fallthrough]];
        case Spacer::Hyphen:
            return 1;
        default:
            return 0;
        }
    }
    return 0; // std::monostate
}

std::string BufferElement::composed() const {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return *elem;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->composed();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::Hyphen:
            return "-";
        case Spacer::Space:
            [[fallthrough]];
        case Spacer::VirtualSpace:
            return " ";
        default:
            return "";
        }
    }

    return ""; // std::monostate
}

std::string BufferElement::converted() const {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        return *elem;
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->converted();
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        switch (*elem) {
        case Spacer::Hyphen:
            return "-";
        case Spacer::Space:
            return " ";
        default:
            return "";
        }
    }

    return ""; // std::monostate
}

DictionaryRow *BufferElement::candidate() const {
    if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->candidate();
    }

    return nullptr;
}

void BufferElement::Erase(SyllableParser *parser, utf8_size_t index) {
    if (auto *elem = std::get_if<Plaintext>(&m_element)) {
        elem->erase(index, 1);
    } else if (auto *elem = std::get_if<TaiText>(&m_element)) {
        elem->Erase(parser, index);
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        m_element = Spacer::None;
    }
}

bool BufferElement::IsVirtualSpace() const {
    if (auto elem = std::get_if<Spacer>(&m_element)) {
        if (*elem == Spacer::VirtualSpace) {
            return true;
        }
    }

    return false;
}

bool BufferElement::IsVirtualSpace(utf8_size_t index) const {
    if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->IsVirtualSpace(index);
    } else if (auto *elem = std::get_if<Spacer>(&m_element)) {
        if (*elem == Spacer::VirtualSpace) {
            return true;
        }
    }

    return false;
}

bool BufferElement::SetKhin(SyllableParser *parser, KhinKeyPosition khin_pos, char khin_key) {
    if (auto *elem = std::get_if<TaiText>(&m_element)) {
        elem->SetKhin(parser, khin_pos, khin_key);
        return true;
    }

    return false;
}

} // namespace khiin::engine

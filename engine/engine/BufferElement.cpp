#include "BufferElement.h"

#include "Models.h"
#include "SyllableParser.h"
#include "common.h"

namespace khiin::engine {
using namespace unicode;

namespace {} // namespace

BufferElement::BufferElement() {}

BufferElement::BufferElement(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

BufferElement::BufferElement(std::string const &elem) {
    m_element.emplace<std::string>(elem);
}

BufferElement::BufferElement(VirtualSpace elem) {
    m_element.emplace<VirtualSpace>(elem);
}

void BufferElement::Replace(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

void BufferElement::Replace(std::string const &elem) {
    m_element.emplace<std::string>(elem);
}

void BufferElement::Replace(VirtualSpace elem) {
    m_element.emplace<VirtualSpace>(elem);
}

utf8_size_t BufferElement::size() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return u8_size(*elem);
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted && elem->candidate()) {
            return u8_size(elem->candidate()->output);
        } else {
            return elem->size();
        }
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return utf8_size_t(1);
    } else { // std::monostate
        return utf8_size_t(0);
    }
}

std::string BufferElement::raw() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->raw();
    } else { // VirtualSpace && std::Monostate
        return std::string();
    }
}

utf8_size_t BufferElement::raw_size() const {
    return unicode::u8_size(raw());
}

utf8_size_t BufferElement::RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const {
    if (raw_caret == 0) {
        return 0;
    }

    if (auto elem = std::get_if<std::string>(&m_element)) {
        return raw_caret;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted && elem->candidate()) {
            return u8_size(elem->candidate()->output);
        } else {
            return elem->RawToComposedCaret(parser, raw_caret);
        }
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return utf8_size_t(1);
    }
    return 0; // std::monostate
}

size_t BufferElement::ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const {
    if (caret == 0) {
        return 0;
    }

    if (auto elem = std::get_if<std::string>(&m_element)) {
        return caret;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted) {
            return elem->ConvertedToRawCaret(parser, caret);
        } else {
            return elem->ComposedToRawCaret(parser, caret);
        }
    }

    return 0; // VirtualSpace && std::monostate
}

std::string BufferElement::composed() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->composed();
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return std::string(1, ' ');
    }

    return std::string(); // std::monostate
}

std::string BufferElement::converted() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->converted();
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return std::string(1, ' ');
    }

    return std::string(); // std::monostate
}

TaiToken *BufferElement::candidate() const {
    if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->candidate();
    }

    return nullptr;
}

void BufferElement::Erase(SyllableParser *parser, utf8_size_t index) {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        safe_erase(*elem, index, 1);
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        elem->Erase(parser, index);
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        elem->erased = true;
    }
}

bool BufferElement::IsVirtualSpace() const {
    if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return true;
    }

    return false;
}

bool BufferElement::IsVirtualSpace(utf8_size_t index) const {
    if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->IsVirtualSpace(index);
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return true;
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

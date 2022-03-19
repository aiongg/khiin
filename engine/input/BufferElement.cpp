#include "BufferElement.h"

#include "data/Models.h"
#include "utils/common.h"

#include "SyllableParser.h"

namespace khiin::engine {
using namespace unicode;

namespace {} // namespace

bool BufferElement::ConvertedEq(BufferElement const &lhs, BufferElement const &rhs) {
    return lhs.converted() == rhs.converted();
}

BufferElement BufferElement::Build(SyllableParser *parser, std::string const &input, TaiToken *match,
                                   bool set_candidate, bool set_converted) {
    auto tt = TaiText::FromMatching(parser, input, match);
    if (set_candidate) {
        tt.SetCandidate(match);
    }
    auto ret = BufferElement(std::move(tt));
    ret.is_converted = set_converted;
    return ret;
}

BufferElement::BufferElement() {}

BufferElement::BufferElement(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

BufferElement::BufferElement(TaiText &&elem) {
    m_element.emplace<TaiText>(std::move(elem));
}

BufferElement::BufferElement(Punctuation const &elem) {
    m_element.emplace<Punctuation>(elem);
}

BufferElement::BufferElement(Punctuation &&elem) {
    m_element.emplace<Punctuation>(std::move(elem));
}

BufferElement::BufferElement(std::string const &elem) {
    m_element.emplace<std::string>(elem);
}

BufferElement::BufferElement(std::string &&elem) {
    m_element.emplace<std::string>(std::move(elem));
}

BufferElement::BufferElement(VirtualSpace elem) {
    m_element.emplace<VirtualSpace>(elem);
}

bool BufferElement::operator==(BufferElement const &rhs) const {
    return BufferElement::ConvertedEq(*this, rhs);
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
    utf8_size_t ret = 0;

    if (auto elem = std::get_if<std::string>(&m_element)) {
        ret = u8_size(*elem);
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted && elem->candidate()) {
            ret = u8_size(elem->candidate()->output);
        } else {
            ret = elem->ComposedSize();
        }
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        ret = u8_size(elem->output);
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        ret = 1;
    }

    return ret;
}

std::string BufferElement::raw() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->RawText();
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        return elem->input;
    } else { // VirtualSpace && std::Monostate
        return std::string();
    }
}

utf8_size_t BufferElement::RawSize() const {
    return unicode::u8_size(raw());
}

utf8_size_t BufferElement::RawToComposedCaret(size_t raw_caret) const {
    if (raw_caret == 0) {
        return 0;
    }

    if (auto elem = std::get_if<std::string>(&m_element)) {
        return raw_caret;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted && elem->candidate()) {
            return u8_size(elem->candidate()->output);
        } else {
            return elem->RawToComposedCaret(raw_caret);
        }
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        return u8_size(elem->output);
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return utf8_size_t(1);
    }
    return 0; // std::monostate
}

size_t BufferElement::ComposedToRawCaret(utf8_size_t caret) const {
    if (caret == 0) {
        return 0;
    }

    if (auto elem = std::get_if<std::string>(&m_element)) {
        return caret;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted) {
            return elem->ConvertedToRawCaret(caret);
        } else {
            return elem->ComposedToRawCaret(caret);
        }
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        return u8_size(elem->input);
    }

    return 0; // VirtualSpace && std::monostate
}

std::string BufferElement::composed() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->ComposedText();
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        return elem->output;
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        return std::string(1, ' ');
    }

    return std::string(); // std::monostate
}

std::string BufferElement::converted() const {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->ConvertedText();
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        return elem->output;
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

void BufferElement::Erase(utf8_size_t index) {
    if (auto elem = std::get_if<std::string>(&m_element)) {
        safe_erase(*elem, index, 1);
    } else if (auto elem = std::get_if<TaiText>(&m_element)) {
        elem->Erase(index);
    } else if (auto elem = std::get_if<Punctuation>(&m_element)) {
        Replace(std::string());
    } else if (auto elem = std::get_if<VirtualSpace>(&m_element)) {
        elem->erased = true;
    }
}

bool BufferElement::IsVirtualSpace() const {
    return std::holds_alternative<VirtualSpace>(m_element);
}

bool BufferElement::IsVirtualSpace(utf8_size_t index) const {
    if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->IsVirtualSpace(index);
    }

    return std::holds_alternative<VirtualSpace>(m_element);
}

bool BufferElement::IsTaiText() const noexcept {
    return std::holds_alternative<TaiText>(m_element);
}

bool BufferElement::SetKhin(KhinKeyPosition khin_pos, char khin_key) {
    if (auto elem = std::get_if<TaiText>(&m_element)) {
        elem->SetKhin(khin_pos, khin_key);
        return true;
    } else if (auto elem = std::get_if<std::string>(&m_element)) {
        if (khin_pos == KhinKeyPosition::Start) {
            elem->insert(0, 2, khin_key);
        } else if (khin_pos == KhinKeyPosition::End) {
            elem->push_back(khin_key);
        }
    }

    return false;
}

} // namespace khiin::engine

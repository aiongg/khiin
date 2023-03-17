#include "BufferElement.h"

#include "data/Models.h"
#include "utils/common.h"

#include "Syllable.h"
#include "SyllableParser.h"
#include "TaiText.h"

namespace khiin::engine {
using namespace unicode;

namespace {} // namespace

bool BufferElement::ConvertedEq(BufferElement const &lhs, BufferElement const &rhs) {
    return lhs.converted() == rhs.converted();
}

BufferElement BufferElement::Build(SyllableParser *parser, std::string const &input, TaiToken *match,
                                   bool set_candidate, bool set_converted) {
    if (match != nullptr && match->id == 0) {
        auto elem = BufferElement(UserToken(input, match));
        elem.SetConverted(set_converted);
        return elem;
    }

    TaiText tt;

    if (match == nullptr) {
        tt = TaiText::FromRawSyllable(parser, input);
    } else {
        tt = TaiText::FromMatching(parser, input, match);
    }

    if (set_candidate) {
        tt.SetCandidate(match);
    }

    auto elem = BufferElement(std::move(tt));
    elem.SetConverted(set_converted);
    return elem;
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

BufferElement::BufferElement(UserToken &&elem) {
    m_element.emplace<UserToken>(std::move(elem));
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
    if (const auto *elem = std::get_if<std::string>(&m_element)) {
        return u8_size(*elem);
    }

    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted && elem->candidate() != nullptr) {
            return u8_size(elem->candidate()->output);
        }

        return elem->ComposedSize();
    }

    if (const auto *elem = std::get_if<Punctuation>(&m_element)) {
        return u8_size(elem->output);
    }

    if (std::holds_alternative<VirtualSpace>(m_element)) {
        return 1;
    }

    if (const auto *elem = std::get_if<UserToken>(&m_element)) {
        if (is_converted && elem->candidate() != nullptr) {
            return elem->OutputSize();
        }

        return elem->InputSize();
    }

    return 0;
}

std::string BufferElement::raw() const {
    if (const auto *elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    }

    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->RawText();
    }

    if (const auto *elem = std::get_if<Punctuation>(&m_element)) {
        return elem->input;
    }

    if (const auto *elem = std::get_if<UserToken>(&m_element)) {
        return elem->Input();
    } // VirtualSpace && std::Monostate

    return std::string();
}

utf8_size_t BufferElement::RawSize() const {
    return u8_size(raw());
}

utf8_size_t BufferElement::RawToComposedCaret(size_t raw_caret) const {
    if (raw_caret == 0) {
        return 0;
    }

    if (std::holds_alternative<std::string>(m_element)) {
        return raw_caret;
    }

    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted && elem->candidate() != nullptr) {
            return u8_size(elem->candidate()->output);
        }

        return elem->RawToComposedCaret(raw_caret);
    }

    if (const auto *elem = std::get_if<Punctuation>(&m_element)) {
        return u8_size(elem->output);
    }

    if (std::holds_alternative<VirtualSpace>(m_element)) {
        return utf8_size_t(1);
    }

    if (const auto *elem = std::get_if<UserToken>(&m_element)) {
        if (is_converted && elem->candidate() != nullptr) {
            return elem->OutputSize();
        }

        return elem->InputSize();
    }

    return 0; // std::monostate
}

size_t BufferElement::ComposedToRawCaret(utf8_size_t caret) const {
    if (caret == 0) {
        return 0;
    }

    if (std::holds_alternative<std::string>(m_element)) {
        return caret;
    }

    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        if (is_converted) {
            return elem->ConvertedToRawCaret(caret);
        }

        return elem->ComposedToRawCaret(caret);
    }

    if (const auto *elem = std::get_if<Punctuation>(&m_element)) {
        return u8_size(elem->input);
    }

    if (const auto *elem = std::get_if<UserToken>(&m_element)) {
        return elem->InputSize();
    }

    return 0; // VirtualSpace && std::monostate
}

std::string BufferElement::composed() const {
    if (const auto *elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    }

    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->ComposedText();
    }

    if (const auto *elem = std::get_if<Punctuation>(&m_element)) {
        return elem->output;
    }

    if (std::holds_alternative<VirtualSpace>(m_element)) {
        return std::string(1, ' ');
    }

    if (const auto *elem = std::get_if<UserToken>(&m_element)) {
        return elem->Input();
    }

    return std::string(); // std::monostate
}

std::string BufferElement::converted() const {
    if (const auto elem = std::get_if<std::string>(&m_element)) {
        return *elem;
    }

    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->ConvertedText();
    }

    if (const auto *elem = std::get_if<Punctuation>(&m_element)) {
        return elem->output;
    }

    if (std::holds_alternative<VirtualSpace>(m_element)) {
        return std::string(1, ' ');
    }

    if (const auto *elem = std::get_if<UserToken>(&m_element)) {
        return elem->Output();
    }

    return std::string(); // std::monostate
}

TaiToken *BufferElement::candidate() const {
    if (auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->candidate();
    }

    return nullptr;
}

void BufferElement::Erase(utf8_size_t index) {
    if (auto *elem = std::get_if<std::string>(&m_element)) {
        safe_erase(*elem, index, 1);
        return;
    }

    if (auto *elem = std::get_if<TaiText>(&m_element)) {
        elem->Erase(index);
        return;
    }

    if (std::holds_alternative<Punctuation>(m_element)) {
        Replace(std::string());
        return;
    }

    if (auto *elem = std::get_if<VirtualSpace>(&m_element)) {
        elem->erased = true;
        return;
    }

    if (auto *elem = std::get_if<UserToken>(&m_element)) {
        auto str = elem->Input();
        safe_erase(str, index, 1);
        Replace(str);
        return;
    }
}

bool BufferElement::IsVirtualSpace() const {
    return std::holds_alternative<VirtualSpace>(m_element);
}

bool BufferElement::IsVirtualSpace(utf8_size_t index) const {
    if (const auto *elem = std::get_if<TaiText>(&m_element)) {
        return elem->IsVirtualSpace(index);
    }

    return std::holds_alternative<VirtualSpace>(m_element);
}

bool BufferElement::IsTaiText() const noexcept {
    return std::holds_alternative<TaiText>(m_element);
}

bool BufferElement::IsUserToken() const noexcept {
    return std::holds_alternative<UserToken>(m_element);
}

bool BufferElement::SetKhin(KhinKeyPosition khin_pos, char khin_key) {
    if (auto *elem = std::get_if<TaiText>(&m_element)) {
        elem->SetKhin(khin_pos, khin_key);
        return true;
    }
    
    if (auto *elem = std::get_if<std::string>(&m_element)) {
        if (khin_pos == KhinKeyPosition::Start) {
            elem->insert(0, 2, khin_key);
        } else if (khin_pos == KhinKeyPosition::End) {
            elem->push_back(khin_key);
        }
    }

    return false;
}

bool BufferElement::IsConverted() const noexcept {
    return is_converted;
}

void BufferElement::SetConverted(bool converted) noexcept {
    is_converted = converted;
}

bool BufferElement::IsSelected() const noexcept {
    return is_selected;
}

void BufferElement::SetSelected(bool selected) noexcept {
    is_selected = selected;
}

} // namespace khiin::engine

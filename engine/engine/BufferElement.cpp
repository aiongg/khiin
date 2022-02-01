#include "BufferElement.h"

#include "SyllableParser.h"
#include "common.h"

namespace khiin::engine {
namespace {

static auto size_of = overloaded //
    {[](Plaintext &elem) {
         return elem.size();
     },
     [](TaiText &elem) {
         return elem.size();
     }};

static auto to_raw = overloaded //
    {[](Plaintext &elem) {
         return elem;
     },
     [](TaiText &elem) {
         return elem.raw();
     }};

} // namespace

BufferElement::BufferElement(TaiText const &elem) {
    m_element.emplace<TaiText>(elem);
}

BufferElement::BufferElement(Plaintext const &elem) {
    m_element.emplace<Plaintext>(elem);
}

utf8_size_t BufferElement::size() {
    return std::visit(size_of, m_element);
}

std::string BufferElement::raw() {
    return std::visit(to_raw, m_element);
}

void BufferElement::RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret) {
    auto to_raw_indexed = overloaded //
        {[&](Plaintext &elem) {
             raw = elem;
             raw_caret = caret;
         },
         [&](TaiText &elem) {
             elem.RawIndexed(caret, raw, raw_caret);
         }};

    std::visit(to_raw_indexed, m_element);
}

utf8_size_t BufferElement::RawToComposedCaret(SyllableParser *parser, size_t raw_caret) {
    auto raw_to_composed_caret = overloaded //
        {[&](Plaintext &elem) {
             return raw_caret;
         },
         [&](TaiText &elem) {
             return elem.RawToComposedCaret(parser, raw_caret);
         }};

    return std::visit(raw_to_composed_caret, m_element);
}

size_t BufferElement::ComposedToRawCaret(SyllableParser* parser, utf8_size_t caret) {
    if (auto elem = std::get_if<Plaintext>(&m_element)) {
        return caret;
    }

    if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->ComposedToRawCaret(parser, caret);
    }
}

std::string BufferElement::composed() {
    if (auto elem = std::get_if<Plaintext>(&m_element)) {
        return *elem;
    }

    if (auto elem = std::get_if<TaiText>(&m_element)) {
        return elem->composed();
    }
}

} // namespace khiin::engine

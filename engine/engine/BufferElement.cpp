#include "BufferElement.h"

#include "common.h"

namespace khiin::engine {
namespace {

static auto size_of = overloaded //
    {[](Plaintext &elem) {
         return elem.size();
     },
     [](BufferSegment &elem) {
         return elem.Size();
     }};

static auto to_raw = overloaded //
    {[](Plaintext &elem) {
         return elem;
     },
     [](BufferSegment &elem) {
         return elem.Raw();
     }};

} // namespace

utf8_size_t BufferElement::Size() {
    return std::visit(size_of, m_element);
}

std::string BufferElement::Raw() {
    return std::visit(to_raw, m_element);
}

void BufferElement::RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret) {
    auto to_raw_indexed = overloaded //
        {[&](Plaintext &elem) {
             raw = elem;
             raw_caret = caret;
         },
         [&](BufferSegment &elem) {
             elem.RawIndexed(caret, raw, raw_caret);
         }};

    std::visit(to_raw_indexed, m_element);
}

} // namespace khiin::engine

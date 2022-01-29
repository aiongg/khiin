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

} //namespace

utf8_size_t BufferElement::Size() {
    return std::visit(size_of, m_element);
}

} // namespace khiin::engine

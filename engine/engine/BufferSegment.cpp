#include "BufferSegment.h"

#include "common.h"

namespace khiin::engine {
namespace {

static auto size_of = overloaded //
    {[](Syllable &elem) {
         return 1;
         //return elem.Size();
     },
     [](Spacer &elem) {
         return 1;
     }};

}

utf8_size_t BufferSegment::Size() {
    auto size = 0;

    for (auto &v_elem : m_elements) {
        if (auto elem = std::get_if<Syllable>(&v_elem)) {
            //size += elem->Size();
        } else {
            // Spacer = 1
            ++size;
        }
    }

    return size;
}

} // namespace khiin::engine

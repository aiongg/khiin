#include "BufferSegment.h"

#include "SyllableParser.h"
#include "common.h"

namespace khiin::engine {
namespace {

static const auto kHyphenStr = std::string(1, '-');
static const auto kSpaceStr = std::string(1, ' ');

std::string get_spacer(Spacer spacer) {
    switch (spacer) {
    case Spacer::Hyphen:
        return kHyphenStr;
    case Spacer::Space:
        return kSpaceStr;
    default:
        return std::string();
    }
}

static auto size_of = overloaded //
    {[](Syllable &elem) {
         return 1;
         // return elem.Size();
     },
     [](Spacer &elem) {
         return 1;
     }};

static auto to_raw = overloaded //
    {[](Syllable &elem) {
         return elem.parser->SerializeRaw(elem);
     },
     [](Spacer &elem) {
         return get_spacer(elem);
     }};

} // namespace

utf8_size_t BufferSegment::Size() {
    auto size = 0;
    for (auto &v_elem : m_elements) {
        size += std::visit(size_of, v_elem);
    }
    return size;
}

std::string BufferSegment::Raw() {
    auto ret = std::string();
    for (auto &v_elem : m_elements) {
        ret += std::visit(to_raw, v_elem);
    }
    return ret;
}

void BufferSegment::RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret) {
    auto remainder = caret;
    auto to_raw_indexed = overloaded //
        {[&](Syllable &elem) {
             elem.parser->SerializeRaw(elem, caret, raw, raw_caret);
         },
         [&](Spacer &elem) {
             auto spacer = get_spacer(elem);
             if (remainder > 0) {
                 remainder -= spacer.size();
                 raw_caret += spacer.size();
             }
         }};
    
    for (auto &v_elem : m_elements) {
        std::visit(to_raw_indexed, v_elem);
    }
}

} // namespace khiin::engine

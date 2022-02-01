#include "BufferSegment.h"

#include "SyllableParser.h"
#include "common.h"
#include "unicode_utils.h"

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
    case Spacer::VirtualSpace:
        [[falthrough]];
    default:
        return std::string();
    }
}

std::string get_virtual_spacer(Spacer spacer) {
    switch (spacer) {
    case Spacer::Hyphen:
        return kHyphenStr;
    case Spacer::Space:
        [[falthrough]];
    case Spacer::VirtualSpace:
        return kSpaceStr;
    default:
        return std::string();
    }
}

static auto size_of = overloaded //
    {[](Syllable &elem) {
         return Utf8Size(elem.composed);
     },
     [](Spacer &elem) {
         return size_t(1);
     }};

static auto to_raw = overloaded //
    {[](Syllable &elem) {
         return elem.raw_input;
     },
     [](Spacer &elem) {
         return get_spacer(elem);
     }};

static auto to_display = overloaded //
    {[](Syllable &elem) {
         return elem.composed;
     },
     [](Spacer &elem) {
         return get_virtual_spacer(elem);
     }};

} // namespace

void BufferSegment::AddItem(Syllable syllable) {
    m_elements.push_back(SegmentElement{});
    m_elements.back().emplace<Syllable>(syllable);
}

void BufferSegment::AddItem(Spacer spacer) {
    m_elements.push_back(SegmentElement{});
    m_elements.back().emplace<Spacer>(spacer);
}

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

std::string BufferSegment::Display() {
    auto ret = std::string();
    for (auto &elem : m_elements) {
        ret += std::visit(to_display, elem);
    }
    return ret;
}

void BufferSegment::RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret) {
    auto remainder = caret;
    auto to_raw_indexed = overloaded //
        {[&](Syllable &elem) {
             elem.raw_input;
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

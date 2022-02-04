#include "TaiText.h"

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
    {[](Syllable const &elem) {
         return Utf8Size(elem.composed);
     },
     [](Spacer elem) {
         return size_t(1);
     }};

static auto to_raw = overloaded //
    {[](Syllable const &elem) {
         return elem.raw_input;
     },
     [](Spacer elem) {
         return get_spacer(elem);
     }};

static auto to_composed = overloaded //
    {[](Syllable const &elem) {
         return elem.composed;
     },
     [](Spacer elem) {
         return get_virtual_spacer(elem);
     }};

} // namespace

void TaiText::AddItem(Syllable syllable) {
    m_elements.push_back(Chunk{});
    m_elements.back().emplace<Syllable>(syllable);
}

void TaiText::AddItem(Spacer spacer) {
    m_elements.push_back(Chunk{});
    m_elements.back().emplace<Spacer>(spacer);
}

void TaiText::SetCandidate(DictionaryRow *candidate) {
    this->candidate = candidate;
}

utf8_size_t TaiText::size() const {
    auto size = 0;
    for (auto &v_elem : m_elements) {
        size += std::visit(size_of, v_elem);
    }
    return size;
}

std::string TaiText::raw() const {
    auto ret = std::string();
    for (auto &v_elem : m_elements) {
        ret += std::visit(to_raw, v_elem);
    }
    return ret;
}

std::string TaiText::composed() const {
    auto ret = std::string();
    for (auto &elem : m_elements) {
        ret += std::visit(to_composed, elem);
    }
    return ret;
}

std::string TaiText::converted() const {
    if (candidate) {
        return candidate->output;
    }

    return composed();
}

void TaiText::RawIndexed(utf8_size_t caret, std::string &raw, size_t &raw_caret) const {
    auto remainder = caret;
    auto to_raw_indexed = overloaded //
        {[&](Syllable const &elem) {
             elem.raw_input;
         },
         [&](Spacer elem) {
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

utf8_size_t TaiText::RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const {
    auto remainder = raw_caret;
    utf8_size_t caret = 0;

    auto raw_to_composed_caret = overloaded //
        {[&](Syllable const &elem) {
             if (remainder > elem.raw_input.size()) {
                 remainder -= elem.raw_input.size();
                 caret += Utf8Size(elem.composed);
             } else {
                 caret += parser->RawToComposedCaret(elem, remainder);
                 remainder = 0;
             }
         },
         [&](Spacer elem) {
             caret += 1;
             if (elem != Spacer::VirtualSpace) {
                 remainder -= 1;
             }
         }};

    for (auto &v_elem : m_elements) {
        if (remainder > 0) {
            std::visit(raw_to_composed_caret, v_elem);
        } else {
            break;
        }
    }

    return caret;
}

size_t TaiText::ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const {
    auto remainder = caret;
    size_t raw_caret = 0;

    auto raw_to_composed_caret = overloaded //
        {[&](Syllable const &elem) {
             if (auto size = Utf8Size(elem.composed); remainder > size) {
                 remainder -= size;
                 raw_caret += elem.raw_input.size();
             } else {
                 raw_caret += parser->ComposedToRawCaret(elem, remainder);
                 remainder = 0;
             }
         },
         [&](Spacer elem) {
             if (elem != Spacer::VirtualSpace) {
                 raw_caret += 1;
             }
             remainder -= 1;
         }};

    for (auto &v_elem : m_elements) {
        if (remainder > 0) {
            std::visit(raw_to_composed_caret, v_elem);
        } else {
            break;
        }
    }

    return raw_caret;
}

void TaiText::Erase(SyllableParser *parser, utf8_size_t index) {
    auto remainder = index;
    auto it = m_elements.begin();
    for (; it != m_elements.end(); ++it) {
        auto size = std::visit(size_of, *it);
        if (remainder > size) {
            remainder -= size;
        } else {
            break;
        }
    }

    if (auto *elem = std::get_if<Spacer>(&*it)) {
        m_elements.erase(it);
    } else if (auto elem = std::get_if<Syllable>(&*it)) {
        parser->Erase(*elem, remainder);
    }
}

bool TaiText::IsVirtualSpace(utf8_size_t index) const {
    auto remainder = index;
    auto it = m_elements.begin();
    for (; it != m_elements.end(); ++it) {
        auto size = std::visit(size_of, *it);
        if (remainder >= size) {
            remainder -= size;
        } else {
            break;
        }
    }

    if (auto *elem = std::get_if<Spacer>(&*it)) {
        return *elem == Spacer::VirtualSpace;
    }

    return false;
}

void TaiText::SetKhin(SyllableParser *parser, KhinKeyPosition khin_pos, char khin_key) {
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (auto elem = std::get_if<Syllable>(&*it)) {
            parser->SetKhin(*elem, khin_pos, khin_key);
        }
    }
}

TaiText TaiText::FromRawSyllable(SyllableParser *parser, std::string const &syllable) {
    Syllable syl;
    parser->ParseRaw(syllable, syl);
    TaiText ret;
    ret.AddItem(syl);
    return ret;
}

} // namespace khiin::engine

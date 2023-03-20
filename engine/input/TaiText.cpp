#include "TaiText.h"

#include <assert.h>

#include "SyllableParser.h"
#include "utils/common.h"
#include "utils/unicode.h"

namespace khiin::engine {
namespace {
namespace u8u = utf8::unchecked;

const auto composed_size_visitor = overloaded  //
    {[](Syllable const &elem) {
         return elem.composed_size();
     },
     [](VirtualSpace elem) {
         return size_t(1);
     }};

const auto to_raw_visitor = overloaded  //
    {[](Syllable const &elem) {
         return elem.raw_input();
     },
     [](VirtualSpace elem) {
         return std::string();
     }};

const auto to_composed_visitor = overloaded  //
    {[](Syllable const &elem) {
         return elem.composed();
     },
     [](VirtualSpace elem) {
         return std::string(1, ' ');
     }};

std::vector<std::string> SplitConvertedToSyllables(std::string_view str) {
    using namespace unicode;
    auto ret = std::vector<std::string>();

    if (str.empty()) {
        return ret;
    }

    auto start = str.begin();
    auto end = str.begin();

    while (end != str.end()) {
        auto cat = glyph_type(start);

        if (cat == GlyphCategory::Hanji) {
            u8u::advance(end, 1);
            ret.push_back(std::string(start, end));
            start = end;
            continue;
        }

        if (cat == GlyphCategory::Alnum) {
            u8u::advance(end, 1);
            while (end != str.end() &&
                   glyph_type(end) == GlyphCategory::Alnum) {
                u8u::advance(end, 1);
            }
            ret.push_back(std::string(start, end));
            start = end;
            continue;
        }
    }

    return ret;
}

}  // namespace

bool TaiText::operator==(TaiText const &rhs) const {
    if (m_elements.size() != rhs.m_elements.size()) {
        return false;
    }

    return ConvertedText() == rhs.ConvertedText();

    for (size_t i = 0; i < m_elements.size(); ++i) {
        auto const &l = m_elements[i];
        auto const &r = rhs.m_elements[i];

        auto const *lvs = std::get_if<VirtualSpace>(&l);
        auto const *rvs = std::get_if<VirtualSpace>(&r);

        if (lvs != nullptr && rvs != nullptr) {
            continue;
        }

        auto const *lsyl = std::get_if<Syllable>(&l);
        auto const *rsyl = std::get_if<Syllable>(&r);

        if (lsyl != nullptr && rsyl != nullptr && *lsyl == *rsyl) {
            continue;
        }

        return false;
    }

    return true;
}

void TaiText::AddItem(Syllable syllable) {
    m_elements.push_back(Chunk{syllable});
}

void TaiText::AddItem(VirtualSpace spacer) {
    m_elements.push_back(Chunk{spacer});
}

void TaiText::SetCandidate(TaiToken const &candidate) {
    this->candidate = candidate;
}

std::string TaiText::RawText() const {
    auto ret = std::string();
    for (auto const &v_elem : m_elements) {
        ret += std::visit(to_raw_visitor, v_elem);
    }
    return ret;
}

utf8_size_t TaiText::RawSize() const {
    utf8_size_t size = 0;
    for (auto const &v_elem : m_elements) {
        if (auto *elem = std::get_if<Syllable>(&v_elem)) {
            size += elem->raw_input_size();
        }
    }
    return size;
}

std::string TaiText::ComposedText() const {
    auto ret = std::string();
    for (auto const &elem : m_elements) {
        ret += std::visit(to_composed_visitor, elem);
    }
    return ret;
}

utf8_size_t TaiText::ComposedSize() const {
    utf8_size_t size = 0;
    for (auto const &v_elem : m_elements) {
        size += std::visit(composed_size_visitor, v_elem);
    }
    return size;
}

std::string TaiText::ConvertedText() const {
    if (candidate) {
        if (Lomaji::IsLomaji(candidate->output)) {
            auto raw = RawText();
            if (!unicode::all_lower(raw)) {
                return Lomaji::MatchCapitalization(raw, candidate->output);
            }
        }

        return candidate->output;
    }

    return ComposedText();
}

utf8_size_t TaiText::ConvertedSize() const {
    utf8_size_t ret = 0;

    return unicode::u8_size(ConvertedText());
}

size_t TaiText::SyllableSize() const {
    size_t ret = 0;
    for (auto const &v_elem : m_elements) {
        if (std::holds_alternative<Syllable>(v_elem)) {
            ++ret;
        }
    }
    return ret;
}

utf8_size_t TaiText::RawToComposedCaret(size_t raw_caret) const {
    auto remainder = raw_caret;
    utf8_size_t caret = 0;

    for (auto const &v_elem : m_elements) {
        if (remainder > 0) {
            if (auto *elem = std::get_if<Syllable>(&v_elem)) {
                if (auto raw_size = elem->raw_input_size();
                    remainder > raw_size) {
                    remainder -= raw_size;
                    caret += elem->composed_size();
                } else {
                    caret += elem->RawToComposedCaret(remainder);
                    remainder = 0;
                }
            } else if (std::holds_alternative<VirtualSpace>(v_elem)) {
                caret += 1;
            }
        } else {
            break;
        }
    }

    return caret;
}

size_t TaiText::ComposedToRawCaret(utf8_size_t caret) const {
    auto remainder = caret;
    size_t raw_caret = 0;

    for (auto const &v_elem : m_elements) {
        if (remainder <= 0) {
            break;
        }

        if (auto *elem = std::get_if<Syllable>(&v_elem)) {
            if (auto size = elem->composed_size(); remainder > size) {
                remainder -= size;
                raw_caret += elem->raw_input_size();
            } else {
                raw_caret += elem->ComposedToRawCaret(remainder);
                remainder = 0;
            }
        }

        if (std::holds_alternative<VirtualSpace>(v_elem)) {
            remainder -= 1;
        }
    }

    return raw_caret;
}

size_t TaiText::ConvertedToRawCaret(utf8_size_t caret) const {
    auto remainder = caret;
    size_t raw_caret = 0;

    if (caret >= ConvertedSize()) {
        return RawSize();
    }

    if (!candidate) {
        return ComposedToRawCaret(caret);
    }

    // ConvertedSyllableSizeMatches();

    for (auto const &v_elem : m_elements) {
        if (remainder <= 0) {
            break;
        }

        if (auto *elem = std::get_if<Syllable>(&v_elem)) {
            if (auto size = elem->composed_size(); remainder > size) {
                remainder -= size;
                raw_caret += elem->raw_input_size();
            } else {
                raw_caret += elem->ComposedToRawCaret(remainder);
                remainder = 0;
            }
        }

        if (std::holds_alternative<VirtualSpace>(v_elem)) {
            remainder -= 1;
        }
    }

    return raw_caret;
}

void TaiText::Erase(utf8_size_t index) {
    auto remainder = index;
    auto it = m_elements.begin();
    auto end = m_elements.end();
    for (; it != end; ++it) {
        auto size = std::visit(composed_size_visitor, *it);
        if (remainder >= size) {
            remainder -= size;
        } else {
            break;
        }
    }

    if (it == end) {
        return;
    }

    if (std::holds_alternative<VirtualSpace>(*it)) {
        m_elements.erase(it);
    } else if (auto *elem = std::get_if<Syllable>(&*it)) {
        elem->Erase(remainder);
    }
}

bool TaiText::IsVirtualSpace(utf8_size_t index) const {
    auto remainder = index;
    auto it = m_elements.begin();
    auto end = m_elements.end();
    for (; it != end; ++it) {
        auto size = std::visit(composed_size_visitor, *it);
        if (remainder >= size) {
            remainder -= size;
        } else {
            break;
        }
    }

    if (it == end) {
        return false;
    }

    return std::holds_alternative<VirtualSpace>(*it);
}

void TaiText::SetKhin(KhinKeyPosition khin_pos, char khin_key) {
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (auto *elem = std::get_if<Syllable>(&*it)) {
            elem->SetKhin(khin_pos, khin_key);
            // Only the first khin can be non-virtual
            khin_pos = KhinKeyPosition::Virtual;
            khin_key = 0;
        }
    }
}

TaiText TaiText::FromRawSyllable(
    SyllableParser *parser, std::string const &syllable) {
    auto syl = parser->ParseRaw(syllable);
    TaiText ret;
    ret.AddItem(syl);
    return ret;
}

TaiText TaiText::FromMatching(
    SyllableParser *parser, std::string const &input, TaiToken const &match) {
    return parser->AsTaiText(input, match.input);
}

}  // namespace khiin::engine

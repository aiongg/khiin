#include "TaiText.h"

#include <assert.h>

#include "SyllableParser.h"
#include "common.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {

static auto composed_size_visitor = overloaded //
    {[](Syllable const &elem) {
         return unicode::u8_size(elem.composed);
     },
     [](VirtualSpace elem) {
         return size_t(1);
     }};

static auto to_raw_visitor = overloaded //
    {[](Syllable const &elem) {
         return elem.raw_input;
     },
     [](VirtualSpace elem) {
         return std::string();
     }};

static auto to_composed_visitor = overloaded //
    {[](Syllable const &elem) {
         return elem.composed;
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
            utf8::unchecked::advance(end, 1);
            ret.push_back(std::string(start, end));
            start = end;
            continue;
        } else if (cat == GlyphCategory::Alnum) {
            utf8::unchecked::advance(end, 1);
            while (end != str.end() && glyph_type(end) == GlyphCategory::Alnum) {
                utf8::unchecked::advance(end, 1);
            }
            ret.push_back(std::string(start, end));
            start = end;
            continue;
        }
    }

    return ret;
}

} // namespace

bool TaiText::operator==(TaiText const &rhs) const {
    if (m_elements.size() != rhs.m_elements.size()) {
        return false;
    }

    for (auto i = 0; i < m_elements.size(); ++i) {
        auto &l = m_elements[i];
        auto &r = rhs.m_elements[i];

        auto lvs = std::get_if<VirtualSpace>(&l);
        auto rvs = std::get_if<VirtualSpace>(&r);

        if (lvs && rvs) {
            continue;
        }

        auto lsyl = std::get_if<Syllable>(&l);
        auto rsyl = std::get_if<Syllable>(&r);

        if (lsyl && rsyl && *lsyl == *rsyl) {
            continue;
        }

        return false;
    }

    return true;
}

void TaiText::AddItem(Syllable syllable) {
    m_elements.push_back(Chunk{});
    m_elements.back().emplace<Syllable>(syllable);
}

void TaiText::AddItem(VirtualSpace spacer) {
    m_elements.push_back(Chunk{});
    m_elements.back().emplace<VirtualSpace>(spacer);
}

void TaiText::SetCandidate(TaiToken *candidate) {
    m_candidate = candidate;
}

std::string TaiText::RawText() const {
    auto ret = std::string();
    for (auto &v_elem : m_elements) {
        ret += std::visit(to_raw_visitor, v_elem);
    }
    return ret;
}

utf8_size_t TaiText::RawSize() const {
    utf8_size_t size = 0;
    for (auto &v_elem : m_elements) {
        if (auto elem = std::get_if<Syllable>(&v_elem)) {
            size += elem->raw_input.size();
        }
    }
    return size;
}

std::string TaiText::ComposedText() const {
    auto ret = std::string();
    for (auto &elem : m_elements) {
        ret += std::visit(to_composed_visitor, elem);
    }
    return ret;
}

utf8_size_t TaiText::ComposedSize() const {
    utf8_size_t size = 0;
    for (auto &v_elem : m_elements) {
        size += std::visit(composed_size_visitor, v_elem);
    }
    return size;
}

std::string TaiText::ConvertedText() const {
    if (m_candidate) {
        return m_candidate->output;
    }

    return ComposedText();
}

utf8_size_t TaiText::ConvertedSize() const {
    utf8_size_t ret = 0;

    return unicode::u8_size(ConvertedText());
}

size_t TaiText::SyllableSize() const {
    size_t ret = 0;
    for (auto &v_elem : m_elements) {
        if (std::holds_alternative<Syllable>(v_elem)) {
            ++ret;
        }
    }
    return ret;
}

TaiToken *TaiText::candidate() const {
    return m_candidate;
}

utf8_size_t TaiText::RawToComposedCaret(SyllableParser *parser, size_t raw_caret) const {
    auto remainder = raw_caret;
    utf8_size_t caret = 0;

    for (auto &v_elem : m_elements) {
        if (remainder > 0) {
            if (auto elem = std::get_if<Syllable>(&v_elem)) {
                if (remainder > elem->raw_input.size()) {
                    remainder -= elem->raw_input.size();
                    caret += unicode::u8_size(elem->composed);
                } else {
                    caret += parser->RawToComposedCaret(*elem, remainder);
                    remainder = 0;
                }
            } else if (auto elem = std::get_if<VirtualSpace>(&v_elem)) {
                caret += 1;
            }
        } else {
            break;
        }
    }

    return caret;
}

size_t TaiText::ComposedToRawCaret(SyllableParser *parser, utf8_size_t caret) const {
    auto remainder = caret;
    size_t raw_caret = 0;

    for (auto &v_elem : m_elements) {
        if (remainder <= 0) {
            break;
        }

        if (auto elem = std::get_if<Syllable>(&v_elem)) {
            if (auto size = unicode::u8_size(elem->composed); remainder > size) {
                remainder -= size;
                raw_caret += elem->raw_input.size();
            } else {
                raw_caret += parser->ComposedToRawCaret(*elem, remainder);
                remainder = 0;
            }
        }

        if (auto elem = std::get_if<VirtualSpace>(&v_elem)) {
            remainder -= 1;
        }
    }

    return raw_caret;
}

size_t TaiText::ConvertedToRawCaret(SyllableParser *parser, utf8_size_t caret) const {
    auto remainder = caret;
    size_t raw_caret = 0;

    if (caret >= ConvertedSize()) {
        return RawSize();
    }

    if (!m_candidate) {
        return ComposedToRawCaret(parser, caret);
    }

    //ConvertedSyllableSizeMatches();

    for (auto &v_elem : m_elements) {
        if (remainder <= 0) {
            break;
        }

        if (auto elem = std::get_if<Syllable>(&v_elem)) {
            if (auto size = unicode::u8_size(elem->composed); remainder > size) {
                remainder -= size;
                raw_caret += elem->raw_input.size();
            } else {
                raw_caret += parser->ComposedToRawCaret(*elem, remainder);
                remainder = 0;
            }
        }

        if (auto elem = std::get_if<VirtualSpace>(&v_elem)) {
            remainder -= 1;
        }
    }

    return raw_caret;
}

void TaiText::Erase(SyllableParser *parser, utf8_size_t index) {
    auto remainder = index;
    auto it = m_elements.begin();
    for (; it != m_elements.end(); ++it) {
        auto size = std::visit(composed_size_visitor, *it);
        if (remainder >= size) {
            remainder -= size;
        } else {
            break;
        }
    }

    if (auto *elem = std::get_if<VirtualSpace>(&*it)) {
        m_elements.erase(it);
    } else if (auto elem = std::get_if<Syllable>(&*it)) {
        parser->Erase(*elem, remainder);
    }
}

bool TaiText::IsVirtualSpace(utf8_size_t index) const {
    auto remainder = index;
    auto it = m_elements.begin();
    for (; it != m_elements.end(); ++it) {
        auto size = std::visit(composed_size_visitor, *it);
        if (remainder >= size) {
            remainder -= size;
        } else {
            break;
        }
    }

    if (auto *elem = std::get_if<VirtualSpace>(&*it)) {
        return true;
    }

    return false;
}

void TaiText::SetKhin(SyllableParser *parser, KhinKeyPosition khin_pos, char khin_key) {
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (auto elem = std::get_if<Syllable>(&*it)) {
            parser->SetKhin(*elem, khin_pos, khin_key);
            // Only the first khin can be non-virtual
            khin_pos = KhinKeyPosition::Virtual;
            khin_key = 0;
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

TaiText TaiText::FromMatching(SyllableParser *parser, std::string const &input, TaiToken *match) {
    auto ret = parser->AsTaiText(input, match->input);
    ret.SetCandidate(match);
    return ret;
}

} // namespace khiin::engine

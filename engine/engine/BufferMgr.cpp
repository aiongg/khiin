#include "BufferMgr.h"

#include "BufferElement.h"
#include "CandidateFinder.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "KhinHandler.h"
#include "Lomaji.h"
#include "Segmenter.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {
using namespace messages;
using namespace unicode;

// Returns size of composed or converted buffer in unicode codepoints
utf8_size_t BufferSize(BufferElementList::iterator const &from, BufferElementList::iterator const &to) {
    auto it = from;
    utf8_size_t size = 0;

    for (; it != to; ++it) {
        size += it->size();
    }

    return size;
}

// Returns size of the raw buffer in unicode codepoints
utf8_size_t RawBufferSize(BufferElementList::iterator const &from, BufferElementList::iterator const &to) {
    auto it = from;
    utf8_size_t size = 0;

    for (; it != to; ++it) {
        size += it->raw_size();
    }

    return size;
}

// Returns iterator pointing to the element at visile caret position
BufferElementList::iterator ElementAtCaret(utf8_size_t caret, BufferElementList &elements) {
    auto rem = caret;
    auto it = elements.begin();
    for (; it != elements.end(); ++it) {
        if (auto size = it->size(); rem > size) {
            rem -= size;
        } else {
            break;
        }
    }
    return it;
}

// Returns iterator pointing to the element at raw caret position
BufferElementList::iterator ElementAtRawCaret(utf8_size_t raw_caret, BufferElementList &elements) {
    auto rem = raw_caret;
    auto it = elements.begin();
    for (; it != elements.end(); ++it) {
        if (auto size = it->raw_size(); rem > size) {
            rem -= size;
        } else {
            break;
        }
    }
    return it;
}

// Returns the raw text between |from| and |to|
std::string GetRawBufferText(BufferElementList::iterator const &from, BufferElementList::iterator const &to) {
    auto it = from;
    auto ret = std::string();
    for (; it != to; ++it) {
        ret.append(it->raw());
    }
    return ret;
}

// Removes all virtual space elements
void RemoveVirtualSpaces(BufferElementList &elements) {
    auto it = std::remove_if(elements.begin(), elements.end(), [](BufferElement const &el) {
        return el.IsVirtualSpace();
    });
    elements.erase(it, elements.end());
}

// Returns true if two words |lhs| and |rhs| need a space between them
// in the standard orthography. In particular, spaces are required between:
// Lomaji and Lomaji, Lomaji and Hanji, and before a khin dot.
bool NeedsVirtualSpace(std::string const &lhs, std::string const &rhs) {
    auto left = end_glyph_type(lhs);
    auto right = start_glyph_type(rhs);

    if (left == GlyphCategory::Alnum && right == GlyphCategory::Alnum ||
        left == GlyphCategory::Alnum && right == GlyphCategory::Khin ||
        left == GlyphCategory::Alnum && right == GlyphCategory::Hanji ||
        left == GlyphCategory::Hanji && right == GlyphCategory::Alnum) {
        return true;
    }
    return false;
}

// Adds virtual spacer elements where required
void AdjustVirtualSpacing(BufferElementList &elements) {
    if (elements.empty()) {
        return;
    }

    RemoveVirtualSpaces(elements);

    for (auto i = elements.size() - 1; i != 0; --i) {
        std::string lhs =
            elements.at(i - 1).is_converted ? elements.at(i - 1).converted() : elements.at(i - 1).composed();
        std::string rhs = elements.at(i).is_converted ? elements.at(i).converted() : elements.at(i).composed();

        if (NeedsVirtualSpace(lhs, rhs)) {
            elements.insert(elements.begin() + i, BufferElement(VirtualSpace()));
        }
    }
}

BufferElementList ConvertWholeBuffer(BufferElementList elements) {
    for (auto &elem : elements) {
        elem.is_converted = true;
    }
    AdjustVirtualSpacing(elements);
    return elements;
}

bool ContainsCandidate(BufferElementList const &a, BufferElementList const &b) {
    auto i = 0;
    auto size = min(a.size(), b.size());
    for (; i < size; ++i) {
        auto &el_a = a[i];
        auto &el_b = b[i];
        if (el_a.converted() != el_b.converted()) {
            return false;
        }
    }
    return true;
}

// Input |caret| is the visible displayed caret, in unicode code points. Returns
// the corresponding raw caret.
size_t GetRawCaret(utf8_size_t caret, BufferElementList &elements, SyllableParser *parser) {
    if (elements.empty()) {
        return 0;
    }

    auto begin = elements.begin();
    auto it = ElementAtCaret(caret, elements);
    auto remainder = caret - BufferSize(begin, it);
    auto raw_remainder = it->ComposedToRawCaret(parser, remainder);
    return RawBufferSize(begin, it) + raw_remainder;
}

// Input |raw_caret| is the raw caret position, in unicode code points. Returns
// the corresponding display caret position.
utf8_size_t SyncCaretFromRaw(size_t raw_caret, BufferElementList &elements, SyllableParser *parser) {
    if (elements.empty()) {
        return 0;
    }
    auto begin = elements.begin();
    auto it = ElementAtRawCaret(raw_caret, elements);
    auto raw_remainder = raw_caret - RawBufferSize(begin, it);
    auto remainder = it->RawToComposedCaret(parser, raw_remainder);
    return BufferSize(begin, it) + remainder;
}

// Returns true if there are any non-converted elements in the compostion
bool HasComposingSection(BufferElementList const &elements) {
    for (auto &el : elements) {
        if (!el.is_converted) {
            return true;
        }
    }

    return false;
}

// Moves converted sections before and after composition
// into separate holders |lhs| and |rhs|. Buffer must be re-joined later.
void IsolateCompositionBuffer(BufferElementList &composing, BufferElementList &lhs, BufferElementList &rhs) {
    auto begin = composing.begin();
    auto it = composing.begin();
    auto end = composing.end();
    while (it != end && it->is_converted) {
        ++it;
    }
    if (it != composing.begin()) {
        lhs.insert(lhs.begin(), begin, it);
        it = composing.erase(composing.begin(), it);
        end = composing.end();
    }
    while (it != end && !it->is_converted) {
        ++it;
    }
    if (it != composing.end()) {
        rhs.insert(rhs.begin(), it, composing.end());
        composing.erase(it, composing.end());
    }
}

// Only call this when the whole buffer is converted. Buffer will be split
// at the caret if the caret element is Hanji, or keep the caret element
// in place if it is Lomaji. Buffer must be re-joined later.
void SplitConvertedBufferForComposition(utf8_size_t caret, BufferElementList &composing, BufferElementList &lhs,
                                        BufferElementList &rhs) {
    auto begin = composing.begin();
    auto elem = ElementAtCaret(caret, composing);

    if (begin != elem) {
        lhs.insert(lhs.begin(), begin, elem);
        elem = composing.erase(begin, elem);
    }

    auto end = composing.end();
    if (elem != end && elem != end - 1) {
        ++elem;
        rhs.insert(rhs.begin(), elem, end);
        composing.erase(elem, end);
    }

    // Only one element remaining: if Hanji we split it and
    // start a new composition buffer, if Lomaji we keep it as-is
    if (auto converted = composing.begin()->converted(); unicode::contains_hanji(converted)) {
        auto remainder = caret - BufferSize(lhs.begin(), lhs.end());
        auto it = converted.begin();
        utf8::unchecked::advance(it, remainder);
        if (it != converted.begin()) {
            auto tmp = BufferElement(std::string(converted.begin(), it));
            tmp.is_converted = true;
            lhs.push_back(std::move(tmp));
        }
        if (it != converted.end()) {
            auto tmp = BufferElement(std::string(it, converted.end()));
            tmp.is_converted = true;
            rhs.insert(rhs.begin(), std::move(tmp));
        }
        composing.clear();
    }
}

class BufferMgrImpl : public BufferMgr {
  public:
    BufferMgrImpl(Engine *engine) : m_engine(engine) {}

    virtual void Clear() override {
        m_composition.clear();
        m_candidates.clear();
        m_caret = 0;
        m_edit_state = EditState::EDIT_EMPTY;
    }

    virtual bool IsEmpty() override {
        return m_composition.empty();
    }

    virtual void Insert(char ch) override {
        if (m_edit_state != EditState::EDIT_COMPOSING) {
            m_edit_state = EditState::EDIT_COMPOSING;
        }

        switch (m_input_mode) {
        case InputMode::Continuous:
            InsertContinuous(ch);
            break;
        default:
            break;
        }
    }

    virtual void MoveCaret(CursorDirection direction) override {
        //if (!HasComposingSection(m_composition)) {
            //MoveFocus(direction);
        //} else {
            MoveCaretBasic(direction);
        //}
    }

    virtual void Erase(CursorDirection direction) override {
        if (direction == CursorDirection::L) {
            MoveCaret(CursorDirection::L);
        }

        auto element = ElementAtCaret(m_caret, m_composition);
        auto caret = m_caret - BufferSize(m_composition.begin(), element);

        if (element->size() == caret) {
            ++element;
            caret = 0;
        }

        if (element->is_converted) {
            EraseConverted(element, caret);
        } else {
            EraseComposing(direction);
        }
    }

    virtual void BuildPreedit(Preedit *preedit) override {
        {
            auto it = m_composition.cbegin();
            auto end = m_composition.cend();
            while (it != end) {
                if ((it != end - 1 && it->IsVirtualSpace() && !it[1].is_converted) || (!it->is_converted)) {
                    auto composing_text = std::string();
                    while (it != end && (it->IsVirtualSpace() || !it->is_converted)) {
                        composing_text += it->composed();
                        ++it;
                    }
                    auto segment = preedit->add_segments();
                    segment->set_status(SegmentStatus::COMPOSING);
                    segment->set_value(composing_text);
                    continue;
                } else if (it->is_converted) {
                    auto segment = preedit->add_segments();
                    segment->set_value(it->converted());
                    if (std::distance(m_composition.cbegin(), it) == m_focused_element) {
                        segment->set_status(SegmentStatus::FOCUSED);
                    } else {
                        segment->set_status(SegmentStatus::CONVERTED);
                    }
                }
                ++it;
            }
        }

        preedit->set_cursor_position(static_cast<int>(m_caret));
    }

    virtual void GetCandidates(messages::CandidateList *candidate_list) override {
        if (m_edit_state == EditState::EDIT_CONVERTED) {
            return;
        }

        auto id = 0;
        for (auto &cand : m_candidates) {
            auto display_str = std::string();
            for (auto &elem : cand) {
                if (elem.is_converted) {
                    display_str += elem.converted();
                } else {
                    display_str += elem.composed();
                }
            }
            auto candidate_output = candidate_list->add_candidates();
            candidate_output->set_value(display_str);
            candidate_output->set_id(id);
            ++id;
        }

        candidate_list->set_focused(static_cast<int32_t>(m_focused_candidate));
    }

    virtual void FocusCandidate(int index) override {}

    virtual void SetInputMode(InputMode new_mode) override {
        m_input_mode = new_mode;
    }

    virtual void SelectNextCandidate() override {
        if (m_input_mode == InputMode::Continuous && m_edit_state == EditState::EDIT_COMPOSING) {
            m_edit_state = EditState::EDIT_CONVERTED;
            SelectCandidate(0);
        } else if (m_edit_state == EDIT_CONVERTED) {
            m_edit_state = EditState::EDIT_SELECTING;
            SelectCandidate((m_focused_candidate + 1) % m_candidates.size());
        } else if (m_edit_state == EditState::EDIT_SELECTING) {
            SelectCandidate((m_focused_candidate + 1) % m_candidates.size());
        }
    }

    virtual EditState edit_state() {
        return m_edit_state;
    }

  private:
    void MoveCaretBasic(CursorDirection direction) {
        auto buffer_text = GetDisplayBuffer();
        m_caret = Lomaji::MoveCaret(buffer_text, m_caret, direction);
    }

    virtual void MoveFocus(CursorDirection direction) override {
        if (m_focused_element == 0 && direction == CursorDirection::L) {
            MoveCaretBasic(direction);
        }
    }

    void InsertContinuous(char ch) {
        SplitBufferForComposition();
        auto raw_composition = GetRawBufferText(m_composition.begin(), m_composition.end());
        auto raw_caret = GetRawCaret(m_caret, m_composition, m_engine->syllable_parser());
        auto it = raw_composition.begin();
        utf8::unchecked::advance(it, raw_caret);
        raw_composition.insert(it, 1, ch);
        ++raw_caret;

        m_composition.clear();
        Segmenter::SegmentText(m_engine, raw_composition, m_composition);
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), m_composition);

        UpdateCandidates(raw_composition);
        JoinBufferAfterComposition(raw_caret);
    }

    void SplitBufferForComposition() {
        if (m_composition.empty()) {
            return;
        }

        if (HasComposingSection(m_composition)) {
            IsolateCompositionBuffer(m_composition, m_precomp, m_postcomp);
        } else {
            SplitConvertedBufferForComposition(m_caret, m_composition, m_precomp, m_postcomp);
        }

        if (!m_precomp.empty()) {
            m_caret -= BufferSize(m_precomp.begin(), m_precomp.end());
        }
    }

    void JoinBufferAfterComposition(utf8_size_t composition_raw_caret) {
        auto raw_caret = RawBufferSize(m_precomp.begin(), m_precomp.end()) + composition_raw_caret;
        if (!m_precomp.empty()) {
            m_composition.insert(m_composition.begin(), m_precomp.begin(), m_precomp.end());
            m_caret += BufferSize(m_precomp.begin(), m_precomp.end());
            m_precomp.clear();
        }
        if (!m_postcomp.empty()) {
            m_composition.insert(m_composition.end(), m_postcomp.begin(), m_postcomp.end());
            m_postcomp.clear();
        }
        AdjustVirtualSpacing(m_composition);
        m_caret = SyncCaretFromRaw(raw_caret, m_composition, m_engine->syllable_parser());
    }

    void UpdateBuffer(std::string const &raw_buffer, size_t raw_caret) {
        std::vector<BufferElement> elements;
        Segmenter::SegmentText(m_engine, raw_buffer, elements);
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), elements);
        m_composition = elements;
        AdjustVirtualSpacing(m_composition);
        m_caret = SyncCaretFromRaw(raw_caret, m_composition, m_engine->syllable_parser());
    }

    void UpdateCandidates(std::string const &raw_buffer) {
        m_candidates.clear();

        if (m_input_mode == InputMode::Continuous && m_edit_state == EditState::EDIT_COMPOSING) {
            m_candidates.push_back(ConvertWholeBuffer(m_composition));
        }

        auto candidates = CandidateFinder::GetCandidatesFromStart(m_engine, nullptr, raw_buffer);

        for (auto &c : candidates) {
            if (!m_candidates.empty() && ContainsCandidate(m_candidates[0], c)) {
                continue;
            }

            m_candidates.push_back(std::move(c));
        }
    }

    std::string GetRawBuffer() {
        return GetRawBufferText(m_composition.begin(), m_composition.end());
    }

    std::string GetDisplayBuffer() {
        auto ret = std::string();
        for (auto &elem : m_composition) {
            if (elem.is_converted) {
                ret.append(elem.converted());
            } else {
                ret.append(elem.composed());
            }
        }
        return ret;
    }

    utf8_size_t GetDisplayBufferSize() {
        return unicode::utf8_size(GetDisplayBuffer());
    }

    void SelectCandidate(size_t index) {
        if (index >= m_candidates.size()) {
            return;
        }

        if (HasComposingSection(m_composition)) {
            IsolateCompositionBuffer(m_composition, m_precomp, m_postcomp);
            if (!m_precomp.empty()) {
                m_caret -= BufferSize(m_precomp.begin(), m_precomp.end());
            }
        }

        auto raw_caret = GetRawCaret(m_caret, m_composition, m_engine->syllable_parser());
        auto &candidate = m_candidates.at(index);
        auto raw_buffer = GetRawBuffer();
        auto candidate_raw = std::string();
        for (auto &el : candidate) {
            candidate_raw += el.raw();
        }
        auto raw_remainder = raw_buffer.substr(candidate_raw.size(), raw_buffer.size());

        m_composition.clear();
        Segmenter::SegmentText(m_engine, raw_remainder, m_composition);
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), m_composition);
        m_composition.insert(m_composition.begin(), candidate.begin(), candidate.end());

        JoinBufferAfterComposition(raw_caret);
        m_focused_candidate = index;
    }

    void EraseConverted(BufferElementList::iterator caret_element, utf8_size_t caret) {
        auto text = caret_element->converted();
        auto end = Lomaji::MoveCaret(text, caret, CursorDirection::R);
        auto from = text.begin();
        auto to = text.begin();
        utf8::unchecked::advance(from, caret);
        utf8::unchecked::advance(to, end);
        text.erase(from, to);

        if (text.empty()) {
            m_composition.erase(caret_element);
        } else {
            caret_element->Replace(text);
        }
    }

    void EraseComposing(CursorDirection direction) {
        auto raw_caret = GetRawCaret(m_caret, m_composition, m_engine->syllable_parser());
        auto remainder = m_caret;

        auto it = m_composition.begin();
        for (; it != m_composition.end(); ++it) {
            if (auto size = it->size(); remainder >= size) {
                remainder -= size;
            } else {
                break;
            }
        }

        if (it->IsVirtualSpace(remainder) && direction == CursorDirection::R) {
            MoveCaret(CursorDirection::R);
            return;
        }

        it->Erase(m_engine->syllable_parser(), remainder);
        if (it->size() == 0) {
            m_composition.erase(it);
        }

        if (IsEmpty()) {
            Clear();
        }

        auto raw_buffer = GetRawBuffer();
        UpdateBuffer(raw_buffer, raw_caret);
    }

    // enum class FocusedElementState {
    //    Focused,
    //    Composing,
    //};

    Engine *m_engine = nullptr;
    utf8_size_t m_caret = 0;
    BufferElementList m_precomp;     // Converted elements before the composition
    BufferElementList m_composition; // Elements in the composition
    BufferElementList m_postcomp;    // Converted elements after the composition
    std::vector<BufferElementList> m_candidates;
    size_t m_focused_candidate = 0;
    // bool m_converted = false;
    int m_focused_element = 0;
    EditState m_edit_state = EditState::EDIT_EMPTY;
    // FocusedElementState m_focused_element_state = FocusedElementState::Composing;
    InputMode m_input_mode = InputMode::Continuous;
};

} // namespace

BufferMgr *BufferMgr::Create(Engine *engine) {
    return new BufferMgrImpl(engine);
}

} // namespace khiin::engine

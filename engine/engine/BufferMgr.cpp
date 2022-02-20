#include "BufferMgr.h"

#include "Buffer.h"
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

class BufferMgrImpl : public BufferMgr {
  public:
    BufferMgrImpl(Engine *engine) : m_engine(engine) {}

    virtual void Clear() override {
        m_composition.Clear();
        m_candidates.clear();
        m_caret = 0;
        m_edit_state = EditState::EDIT_EMPTY;
        NavMode m_nav_mode = NavMode::ByCharacter;
        m_focused_candidate = 0;
        m_focused_element = 0;

        m_composition_copy.Clear();
        m_caret_copy = 0;
    }

    virtual bool IsEmpty() override {
        return m_composition.Empty();
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
        if (m_edit_state == EditState::EDIT_COMPOSING) {
            MoveCaretBasic(direction);
        } else if (m_edit_state == EditState::EDIT_CONVERTED) {
            MoveFocus(direction);
        }
    }

    virtual void Erase(CursorDirection direction) override {
        if (direction == CursorDirection::L) {
            MoveCaret(CursorDirection::L);
        }

        auto elem_it = m_composition.IterCaret(m_caret);
        auto caret = m_caret - Buffer::TextSize(m_composition.Begin(), elem_it);

        if (elem_it->size() == caret) {
            ++elem_it;
            caret = 0;
        }

        if (elem_it->is_converted) {
            EraseConverted(elem_it, caret);
        } else {
            EraseComposing(direction);
        }
    }

    virtual void BuildPreedit(Preedit *preedit) override {
        {
            auto it = m_composition.CBegin();
            auto end = m_composition.CEnd();
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
                    if (std::distance(m_composition.CBegin(), it) == m_focused_element) {
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
            auto candidate_output = candidate_list->add_candidates();
            candidate_output->set_value(cand.Text());
            candidate_output->set_id(id);
            ++id;
        }

        candidate_list->set_focused(static_cast<int32_t>(m_focused_candidate));
    }

    virtual void SetInputMode(InputMode new_mode) override {
        m_input_mode = new_mode;
    }

    virtual void HandleSelectOrFocus() override {
        if (m_input_mode == InputMode::Continuous && m_edit_state == EditState::EDIT_COMPOSING) {
            m_edit_state = EditState::EDIT_CONVERTED;
            SelectCandidate(0);
        } else if (m_edit_state == EDIT_CONVERTED) {
            m_edit_state = EditState::EDIT_SELECTING;
            FocusNextCandidate();
        } else if (m_edit_state == EditState::EDIT_SELECTING) {
            FocusNextCandidate();
        }
    }

    virtual void FocusNextCandidate() override {
        if (m_edit_state == EditState::EDIT_COMPOSING) {
            m_edit_state = EditState::EDIT_SELECTING;
            FocusCandidate(0);
        } else {
            m_edit_state = EditState::EDIT_SELECTING;
            FocusCandidate((m_focused_candidate + 1) % m_candidates.size());
        }
    }

    virtual void FocusPrevCandidate() override {
        m_edit_state = EditState::EDIT_SELECTING;

        if (m_focused_candidate == 0) {
            FocusCandidate(m_candidates.size() - 1);
        } else {
            FocusCandidate(m_focused_candidate - 1);
        }
    }

    virtual void FocusCandidate(size_t index) override {
        assert(index < m_candidates.size());

        if (m_composition_copy.Empty()) {
            SaveBufferState();
        } else {
            RestoreSavedBufferState();
        }

        FocusCandidate_(index);
    }

    virtual void SelectCandidate(size_t index) {
        m_nav_mode = NavMode::BySegment;
        SelectCandidate_(index);
    }

    virtual EditState edit_state() {
        return m_edit_state;
    }

  private:
    void MoveCaretBasic(CursorDirection direction) {
        auto buffer_text = GetDisplayBuffer();
        m_caret = Lomaji::MoveCaret(buffer_text, m_caret, direction);
        FocusElementAtCursor();
    }

    void FocusElementAtCursor() {
        auto focus_idx = std::distance(m_composition.Begin(), m_composition.IterCaret(m_caret));
        if (focus_idx != m_focused_element) {
            OnFocusElementChange(focus_idx);
        }
    }

    void OnFocusElementChange(size_t new_focused_element_idx) {
        m_focused_element = new_focused_element_idx;

        if (m_edit_state == EditState::EDIT_CONVERTED) {
            UpdateCandidatesForFocusedElement();
        }
    }

    void UpdateCandidatesForFocusedElement() {
        auto raw_text = Buffer::RawText(m_composition.Begin() + m_focused_element, m_composition.End());
        m_candidates = CandidateFinder::GetCandidatesFromStart(m_engine, nullptr, raw_text);
    }

    virtual void MoveFocus(CursorDirection direction) override {
        if (m_nav_mode == NavMode::BySegment && m_focused_element == 0 && direction == CursorDirection::L) {
            m_nav_mode = NavMode::ByCharacter;
        }

        if (m_nav_mode == NavMode::BySegment) {
            auto it = m_composition.Begin() + m_focused_element;

            if (direction == CursorDirection::R) {
                if (m_focused_element >= m_composition.get().size() - 1) {
                    return;
                }

                ++it;
                while (it->IsVirtualSpace()) {
                    ++it;
                }
            } else if (direction == CursorDirection::L) {

                --it;
                while (it->IsVirtualSpace()) {
                    --it;
                }
            }

            auto idx = static_cast<int>(std::distance(m_composition.Begin(), it));
            if (idx != m_focused_element) {
                OnFocusElementChange(idx);
            }
        } else if (m_nav_mode == NavMode::ByCharacter) {
            MoveCaretBasic(direction);
        }
    }

    void InsertContinuous(char ch) {
        SplitBufferForComposition();
        auto raw_composition = m_composition.RawText();
        auto raw_caret = m_composition.RawCaretFrom(m_caret, m_engine->syllable_parser());
        auto it = raw_composition.begin();
        utf8::unchecked::advance(it, raw_caret);
        raw_composition.insert(it, 1, ch);
        ++raw_caret;

        m_composition.Clear();
        m_candidates.clear();
        Segmenter::SegmentText(m_engine, raw_composition, m_composition.get());
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), m_composition.get());
        m_candidates = CandidateFinder::ContinuousCandidates(m_engine, nullptr, raw_composition);
        raw_caret += m_precomp.RawTextSize();
        m_composition.Join(&m_precomp, &m_postcomp);
        m_composition.AdjustVirtualSpacing();
        m_caret = m_composition.CaretFrom(raw_caret, m_engine->syllable_parser());
    }

    void SplitBufferForComposition() {
        if (m_composition.Empty()) {
            return;
        }

        if (m_composition.HasComposing()) {
            m_composition.IsolateComposing(m_precomp, m_postcomp);
        } else {
            m_composition.SplitForComposition(m_caret, m_precomp, m_postcomp);
        }

        if (!m_precomp.Empty()) {
            m_caret -= m_precomp.TextSize();
        }
    }

    void FocusCandidate_(size_t index) {
        auto raw_caret = m_composition.RawCaretFrom(m_caret, m_engine->syllable_parser());
        Buffer candidate = m_candidates.at(index);
        m_composition.SplitAtElement(m_focused_element, &m_precomp, nullptr);

        m_focused_element = 0;

        auto replace_from = m_composition.Begin();
        auto raw_cand_size = candidate.RawTextSize();

        auto replace_to = m_composition.IterRawCaret(raw_cand_size) + 1;
        auto raw_buf_size = Buffer::RawTextSize(replace_from, replace_to);

        assert(raw_buf_size >= raw_cand_size);

        if (raw_buf_size > raw_cand_size) {
            auto raw_text = Buffer::RawText(replace_from, replace_to);
            auto raw_it = raw_text.begin() + raw_buf_size - (raw_buf_size - raw_cand_size);
            auto deconverted = std::string(raw_it, raw_text.end());

            if (replace_to != m_composition.End()) {
                ++replace_to;
            }

            while (replace_to != m_composition.End() &&
                   (!CandidateFinder::HasExactMatch(m_engine, deconverted) || deconverted.empty())) {
                deconverted.append(replace_to->raw());
                ++replace_to;
            }

            auto next_buf = CandidateFinder::ContinuousBestMatch(m_engine, nullptr, deconverted);
            next_buf.SetConverted(false);
            candidate.Append(next_buf);

            auto cand_raw_size = candidate.RawTextSize();
            auto pre_raw_size = m_precomp.RawTextSize();
            raw_caret = max(raw_caret, cand_raw_size + pre_raw_size);
        }

        m_composition.Replace(replace_from, replace_to, candidate);
        m_focused_element = m_precomp.get().size();
        m_focused_candidate = index;
        m_composition.Join(&m_precomp, &m_postcomp);
        m_composition.AdjustVirtualSpacing();
        m_caret = m_composition.CaretFrom(raw_caret, m_engine->syllable_parser());
    }

    void SelectCandidate_(size_t index) {
        if (index >= m_candidates.size()) {
            return;
        }

        if (m_composition.HasComposing()) {
            m_composition.IsolateComposing(m_precomp, m_postcomp);

            if (!m_precomp.Empty()) {
                m_caret -= m_precomp.TextSize();
                m_focused_element = 0;
            }
        }

        auto raw_caret = m_composition.RawCaretFrom(m_caret, m_engine->syllable_parser());
        auto &candidate = m_candidates.at(index);
        auto raw_buffer = GetRawBuffer();
        auto candidate_raw = candidate.RawText();
        auto raw_remainder = std::string(raw_buffer.cbegin() + candidate_raw.size(), raw_buffer.cend());

        m_composition.Clear();
        Segmenter::SegmentText(m_engine, raw_remainder, m_composition.get());
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), m_composition.get());
        m_composition.get().insert(m_composition.Begin(), candidate.Begin(), candidate.End());

        m_focused_element = m_precomp.get().size();
        raw_caret += m_precomp.RawTextSize();
        m_composition.Join(&m_precomp, &m_postcomp);
        m_composition.AdjustVirtualSpacing();
        m_caret = m_composition.CaretFrom(raw_caret, m_engine->syllable_parser());

        m_focused_candidate = 0;
        m_candidates = CandidateFinder::GetCandidatesFromStart(m_engine, nullptr, raw_buffer);
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
            m_composition.get().erase(caret_element);
        } else {
            caret_element->Replace(text);
        }
    }

    void EraseComposing(CursorDirection direction) {
        auto raw_caret = m_composition.RawCaretFrom(m_caret, m_engine->syllable_parser());
        auto remainder = m_caret;

        auto it = m_composition.Begin();
        for (; it != m_composition.End(); ++it) {
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
            m_composition.get().erase(it);
        }

        if (IsEmpty()) {
            Clear();
        }

        auto raw_buffer = GetRawBuffer();
        std::vector<BufferElement> elements;
        Segmenter::SegmentText(m_engine, raw_buffer, elements);
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), elements);
        m_composition = Buffer(std::move(elements));
        m_composition.AdjustVirtualSpacing();
        m_caret = m_composition.CaretFrom(raw_caret, m_engine->syllable_parser());
    }

    std::string GetRawBuffer() {
        return m_composition.RawText();
    }

    std::string GetDisplayBuffer() {
        return m_composition.Text();
    }

    utf8_size_t GetDisplayBufferSize() {
        return u8_size(GetDisplayBuffer());
    }

    void SaveBufferState() {
        m_composition_copy = m_composition;
        m_caret_copy = m_caret;
    }

    void RestoreSavedBufferState() {
        m_composition = m_composition_copy;
        m_caret = m_caret_copy;
    }

    enum class NavMode {
        ByCharacter,
        BySegment,
    };

    Engine *m_engine = nullptr;
    utf8_size_t m_caret = 0;
    Buffer m_composition;      // Elements in the composition
    Buffer m_precomp;          // Converted elements before the composition
    Buffer m_postcomp;         // Converted elements after the composition
    std::vector<Buffer> m_candidates;
    size_t m_focused_candidate = 0;
    size_t m_focused_element = 0;
    EditState m_edit_state = EditState::EDIT_EMPTY;
    InputMode m_input_mode = InputMode::Continuous;
    NavMode m_nav_mode = NavMode::ByCharacter;

    Buffer m_composition_copy; // Used when focusing candidates
    utf8_size_t m_caret_copy = 0;
};

} // namespace

BufferMgr *BufferMgr::Create(Engine *engine) {
    return new BufferMgrImpl(engine);
}

} // namespace khiin::engine

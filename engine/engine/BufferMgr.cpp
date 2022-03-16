#include "BufferMgr.h"

#include <algorithm>

#include "proto/proto.h"

#include "Buffer.h"
#include "BufferElement.h"
#include "CandidateFinder.h"
#include "Config.h"
#include "Engine.h"
#include "KeyConfig.h"
#include "KhinHandler.h"
#include "Lomaji.h"
#include "unicode_utils.h"

namespace khiin::engine {
namespace {
using namespace unicode;

enum class EditState {
    Empty,
    Composing,
    Converted,
    Selecting,
};

enum class NavMode {
    BySegment,
    ByCharacter,
};

class BufferMgrImpl : public BufferMgr {
  public:
    BufferMgrImpl(Engine *engine) : m_engine(engine) {}

  private:
    //+---------------------------------------------------------------------------
    //
    // Protobuf-related methods
    //
    //----------------------------------------------------------------------------

    virtual void BuildPreedit(proto::Preedit *preedit) override {
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
                    segment->set_status(proto::SS_COMPOSING);
                    segment->set_value(composing_text);
                    continue;
                } else if (it->IsVirtualSpace()) {
                    auto segment = preedit->add_segments();
                    segment->set_value(" ");
                    segment->set_status(proto::SS_UNMARKED);
                } else if (it->is_converted) {
                    auto segment = preedit->add_segments();
                    segment->set_value(it->converted());
                    if (std::distance(m_composition.CBegin(), it) == m_focused_element) {
                        segment->set_status(proto::SS_FOCUSED);
                    } else {
                        segment->set_status(proto::SS_CONVERTED);
                    }
                }
                ++it;
            }
        }

        preedit->set_cursor_position(static_cast<int32_t>(m_caret));
    }

    virtual void GetCandidates(proto::CandidateList *candidate_list) override {
        AdjustAllCandidates();

        if (m_edit_state == EditState::Converted) {
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

    virtual proto::EditState edit_state() {
        switch (m_edit_state) {
        case EditState::Composing:
            return proto::ES_COMPOSING;
        case EditState::Converted:
            return proto::ES_CONVERTED;
        case EditState::Selecting:
            return proto::ES_SELECTING;
        default:
            return proto::ES_EMPTY;
        }
    }

    //+---------------------------------------------------------------------------
    //
    // BufferMgr public interface
    //
    //----------------------------------------------------------------------------

    virtual bool IsEmpty() override {
        return m_composition.Empty();
    }

    virtual void Clear() override {
        m_composition.Clear();
        m_candidates.clear();
        m_caret = 0;
        m_edit_state = EditState::Empty;
        NavMode m_nav_mode = NavMode::ByCharacter;
        m_focused_candidate = 0;
        m_focused_element = 0;
    }

    virtual void Insert(char ch) override {
        if (m_edit_state != EditState::Composing) {
            m_edit_state = EditState::Composing;
        }

        switch (config()->input_mode()) {
        case InputMode::Continuous:
            InsertContinuous(ch);
            break;
        case InputMode::Basic:
            InsertBasic(ch);
            break;
        case InputMode::Manual:
            InsertManual(ch);
            break;
        default:
            break;
        }
    }

    virtual void Erase(CursorDirection direction) override {
        if (direction == CursorDirection::L) {
            MoveCaret(CursorDirection::L);
        }

        auto elem_it = m_composition.IterCaret(m_caret);

        if (elem_it->is_converted) {
            auto pos_in_elem = m_caret - Buffer::TextSize(m_composition.Begin(), elem_it);

            if (elem_it->size() == pos_in_elem) {
                ++elem_it;
                pos_in_elem = 0;
            }

            EraseConverted(elem_it, pos_in_elem);
        } else {
            EraseComposing(direction);
        }

        if (m_focused_element > m_composition.Size() - 1) {
            OnFocusElementChange(m_composition.Size() - 1);
        }
    }

    virtual void HandleLeftRight(CursorDirection direction) override {
        if (m_edit_state == EditState::Composing) {
            MoveCaret(direction);
        } else if (m_edit_state == EditState::Converted) {
            MoveFocusOrCaret(direction);
        }
    }

    virtual bool HandleSelectOrCommit() override {
        if (m_edit_state == EditState::Selecting) {
            SelectCandidate(m_focused_candidate);
            return false;
        }

        return true;
    }

    virtual void HandleSelectOrFocus() override {
        if (m_edit_state == EditState::Composing) {
            m_edit_state = EditState::Converted;
            SelectCandidate(0);
        } else if (m_edit_state == EditState::Converted) {
            m_edit_state = EditState::Selecting;
            FocusNextCandidate();
        } else if (m_edit_state == EditState::Selecting) {
            FocusNextCandidate();
        }
    }

    virtual void FocusNextCandidate() override {
        if (m_edit_state == EditState::Composing) {
            m_edit_state = EditState::Selecting;
            FocusCandidate(0);
        } else {
            m_edit_state = EditState::Selecting;
            if (m_focused_candidate >= m_candidates.size() - 1) {
                FocusCandidate(0);
            } else {
                FocusCandidate((m_focused_candidate + 1));
            }
        }
    }

    virtual void FocusPrevCandidate() override {
        m_edit_state = EditState::Selecting;

        if (m_focused_candidate == 0) {
            FocusCandidate(m_candidates.size() - 1);
        } else {
            FocusCandidate(m_focused_candidate - 1);
        }
    }

    virtual void FocusCandidate(size_t index) override {
        if (m_candidates.empty()) {
            m_edit_state = EditState::Converted;
            return;
        }

        assert(index < m_candidates.size());

        FocusCandidate_(index);
    }

    virtual void SelectCandidate(size_t index) {
        SelectCandidate_(index);
        m_nav_mode = NavMode::BySegment;
        m_edit_state = EditState::Converted;
    }

    //+---------------------------------------------------------------------------
    //
    // BufferMgr private methods
    //
    //----------------------------------------------------------------------------

  private:
    //+----------------------------------
    // Navigation
    //

    void MoveCaret(CursorDirection direction) {
        auto buffer_text = GetDisplayBuffer();
        m_caret = Lomaji::MoveCaret(buffer_text, m_caret, direction);

        if (m_edit_state != EditState::Composing) {
            FocusElementAtCursor();
        }
    }

    void FocusElementAtCursor() {
        auto focus_idx = std::distance(m_composition.Begin(), m_composition.IterCaret(m_caret));
        if (focus_idx != m_focused_element) {
            OnFocusElementChange(focus_idx);
        }
    }

    void OnFocusElementChange(size_t new_focused_element_idx) {
        m_focused_element = new_focused_element_idx;

        auto elem = m_composition.Begin() + m_focused_element;
        while (elem != m_composition.End() && elem->IsVirtualSpace()) {
            ++elem;
            ++m_focused_element;
        }

        if (m_edit_state == EditState::Converted) {
            UpdateCandidatesForFocusedElement();
        }
    }

    void UpdateCandidatesForFocusedElement() {
        auto raw_text = Buffer::RawText(m_composition.Begin() + m_focused_element, m_composition.End());

        switch (config()->input_mode()) {
        case InputMode::Continuous:
            m_candidates = CandidateFinder::GetCandidatesFromStart(m_engine, nullptr, raw_text);
            break;
        case InputMode::Basic:
            m_candidates = CandidateFinder::LongestCandidatesFromStart(m_engine, nullptr, raw_text);
            break;
        }

        SetFocusedCandidateIndexToCurrent();
    }

    void MoveFocusOrCaret(CursorDirection direction) {
        if (m_nav_mode == NavMode::BySegment && m_focused_element == 0 && direction == CursorDirection::L) {
            m_nav_mode = NavMode::ByCharacter;
        }

        if (m_nav_mode == NavMode::BySegment) {
            MoveFocus(direction);
        } else if (m_nav_mode == NavMode::ByCharacter) {
            MoveCaret(direction);
        }
    }

    void MoveFocus(CursorDirection direction) {
        auto it = m_composition.Begin() + m_focused_element;

        if (direction == CursorDirection::R) {
            if (m_focused_element >= m_composition.Size() - 1) {
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
    }

    //+----------------------------------
    // Composition
    //

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

    void JoinBufferUpdateCaretAndFocus(utf8_size_t raw_caret) {
        // After joining, focus moves to the first element past m_precomp
        // Save this position as a virtual raw caret, since its actual
        // position may be affected by virtual spacing
        auto focus_raw_caret = m_precomp.RawTextSize() + 1;

        // Join the elements and adjust spacing
        m_composition.Join(&m_precomp, &m_postcomp);
        KhinHandler::AutokhinBuffer(m_engine, m_composition.get());
        m_composition.AdjustVirtualSpacing();

        // Get the focused element using the virtual raw caret position
        auto focus_elem_it = m_composition.IterRawCaret(focus_raw_caret);
        OnFocusElementChange(std::distance(m_composition.Begin(), focus_elem_it));
        m_caret = m_composition.CaretFrom(raw_caret);
    }

    std::pair<std::string, size_t> BeginInsertion(char ch) {
        SplitBufferForComposition();
        auto raw_composition = m_composition.RawText();
        auto raw_caret = m_composition.RawCaretFrom(m_caret);
        auto it = raw_composition.begin();
        utf8::unchecked::advance(it, raw_caret);
        raw_composition.insert(it, 1, ch);
        ++raw_caret;

        return std::make_pair(raw_composition, raw_caret);
    }

    void FinalizeInsertion(size_t raw_caret) {
        m_composition.SetConverted(false);
        raw_caret += m_precomp.RawTextSize();
        JoinBufferUpdateCaretAndFocus(raw_caret);
    }

    void SetCompositionAndCandidatesContinuous(std::string const &raw_composition) {
        m_candidates = CandidateFinder::MultiSegmentCandidates(m_engine, nullptr, raw_composition);
        m_composition = m_candidates[0];
        m_composition.SetConverted(false);
        assert(m_composition.RawText() == raw_composition);
    }

    void SetCompositionAndCandidatesBasic(std::string const &raw_composition) {
        m_candidates = CandidateFinder::LongestCandidatesFromStart(m_engine, nullptr, raw_composition);
        m_composition = m_candidates[0];
        m_composition.SetConverted(false);

        auto raw_comp_size = u8_size(raw_composition);
        auto top_cand_size = m_composition.RawTextSize();

        if (raw_comp_size > top_cand_size) {
            m_composition.Append(raw_composition.substr(top_cand_size, raw_comp_size));
        }

        assert(m_composition.RawText() == raw_composition);
    }

    void InsertContinuous(char ch) {
        auto [raw_composition, raw_caret] = BeginInsertion(ch);
        SetCompositionAndCandidatesContinuous(raw_composition);
        FinalizeInsertion(raw_caret);
    }

    void InsertBasic(char ch) {
        auto [raw_composition, raw_caret] = BeginInsertion(ch);
        SetCompositionAndCandidatesBasic(raw_composition);
        FinalizeInsertion(raw_caret);
    }

    void InsertManual(char ch) {}

    //+----------------------------------
    // Candidate navigation & selection
    //

    size_t FocusCandidate_(size_t index, bool select = false) {
        auto raw_caret = m_composition.RawCaretFrom(m_caret);
        Buffer candidate = m_candidates.at(index);

        Buffer tmp = candidate;
        KhinHandler::AutokhinBuffer(m_engine, tmp.get());
        tmp.AdjustVirtualSpacing();
        auto candidate_size = tmp.Size();

        if (select) {
            for (auto &ea : candidate.get()) {
                ea.is_selected = true;
            }
        }

        m_composition.SplitAtElement(m_focused_element, &m_precomp, nullptr);

        auto replace_from = m_composition.Begin();
        auto raw_cand_size = candidate.RawTextSize();

        auto replace_to = m_composition.IterRawCaret(raw_cand_size) + 1;
        auto raw_buf_size = Buffer::RawTextSize(replace_from, replace_to);

        assert(raw_buf_size >= raw_cand_size);

        if (raw_buf_size > raw_cand_size) {
            auto raw_text = Buffer::RawText(replace_from, replace_to);
            auto deconverted = std::string(raw_text.begin() + raw_cand_size, raw_text.end());

            while (replace_to != m_composition.End() &&
                   (!CandidateFinder::HasExactMatch(m_engine, deconverted) || deconverted.empty())) {
                deconverted.append(replace_to->raw());
                ++replace_to;
            }

            auto next_buf = CandidateFinder::ContinuousBestMatch(m_engine, nullptr, deconverted);

            if (next_buf.Empty()) {
                next_buf.Append(std::move(deconverted));
            }

            next_buf.SetConverted(false);
            candidate.Append(next_buf);

            auto cand_raw_size = candidate.RawTextSize();
            auto pre_raw_size = m_precomp.RawTextSize();
            raw_caret = (std::max)(raw_caret, cand_raw_size + pre_raw_size);
        }

        auto it = m_composition.Replace(replace_from, replace_to, candidate);
        it += candidate.Size();

        while (it != m_composition.End() && !it->is_selected) {
            it->is_converted = false;
            ++it;
        }

        m_focused_candidate = index;

        JoinBufferUpdateCaretAndFocus(raw_caret);
        return candidate_size;
    }

    void SelectCandidate_(size_t index) {
        assert(index < m_candidates.size());
        
        // Move the candidate into the composition
        auto n_elems_added = FocusCandidate_(index, true);

        // When |EditState::Selecting|, buffer focus moves to the next element
        auto comp_begin = m_composition.Begin();
        auto focused_elem = comp_begin + m_focused_element;
        auto comp_end = m_composition.End();
        size_t n_elems_remaining = std::distance(focused_elem, comp_end);

        if (m_edit_state == EditState::Selecting && n_elems_remaining > n_elems_added) {
            focused_elem += n_elems_added;
        }

        // Don't let a virtual space become focused
        while (focused_elem != comp_end && focused_elem->IsVirtualSpace()) {
            ++focused_elem;
        }

        m_focused_element = std::distance(comp_begin, focused_elem);
        UpdateCandidatesForFocusedElement();
    }

    void SetFocusedCandidateIndexToCurrent() {
        auto &current = m_composition.At(m_focused_element);

        for (auto i = 0; i < m_candidates.size(); ++i) {
            auto &check = m_candidates.at(i).At(0);

            if (current == check) {
                m_focused_candidate = i;
                return;
            }
        }

        // Should not reach here
        assert("Current buffer element is not a candidate");
    }

    //+----------------------------------
    // Erasing
    //

    void EraseConverted(BufferElementList::iterator caret_element, utf8_size_t caret) {
        auto text = caret_element->converted();
        auto end = Lomaji::MoveCaret(text, caret, CursorDirection::R);
        auto from = text.begin();
        auto to = text.begin();
        utf8::unchecked::advance(from, caret);
        utf8::unchecked::advance(to, end);
        text.erase(from, to);

        if (text.empty()) {
            m_composition.Erase(caret_element);
        } else {
            caret_element->Replace(text);
        }
    }

    void EraseComposing(CursorDirection direction) {
        auto raw_caret = m_composition.RawCaretFrom(m_caret);
        SplitBufferForComposition();

        auto pos = m_caret;

        auto it = m_composition.Begin();
        for (; it != m_composition.End(); ++it) {
            if (auto size = it->size(); pos >= size) {
                pos -= size;
            } else {
                break;
            }
        }

        if (it->IsVirtualSpace(pos) && direction == CursorDirection::R) {
            HandleLeftRight(CursorDirection::R);
            return;
        }

        it->Erase(pos);
        if (it->size() == 0) {
            m_composition.Erase(it);
        }

        if (IsEmpty()) {
            Clear();
        }

        switch (config()->input_mode()) {
        case InputMode::Continuous:
            SetCompositionAndCandidatesContinuous(GetRawBuffer());
            break;
        case InputMode::Basic:
            SetCompositionAndCandidatesBasic(GetRawBuffer());
            break;
        }

        JoinBufferUpdateCaretAndFocus(raw_caret);
    }

    void AdjustAllCandidates() {
        if (m_candidates.empty()) {
            return;
        }

        for (auto &cand : m_candidates) {
            KhinHandler::AutokhinBuffer(m_engine, cand.get());
            cand.AdjustVirtualSpacing();
        }
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

    Config *config() {
        return m_engine->config();
    }

    SyllableParser *parser() {
        return m_engine->syllable_parser();
    }

    enum class NavMode {
        ByCharacter,
        BySegment,
    };

    Engine *m_engine = nullptr;
    utf8_size_t m_caret = 0;
    Buffer m_composition; // Elements in the composition
    Buffer m_precomp;     // Converted elements before the composition
    Buffer m_postcomp;    // Converted elements after the composition
    std::vector<Buffer> m_candidates;
    size_t m_focused_candidate = 0;
    size_t m_focused_element = 0;
    EditState m_edit_state = EditState::Empty;
    NavMode m_nav_mode = NavMode::ByCharacter;
};

} // namespace

BufferMgr *BufferMgr::Create(Engine *engine) {
    return new BufferMgrImpl(engine);
}

} // namespace khiin::engine

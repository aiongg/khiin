#include "BufferMgr.h"

#include <algorithm>

#include "config/Config.h"
#include "config/KeyConfig.h"
#include "data/Dictionary.h"
#include "proto/proto.h"
#include "utils/unicode.h"

#include "Buffer.h"
#include "BufferElement.h"
#include "CandidateFinder.h"
#include "Engine.h"
#include "KhinHandler.h"
#include "Lomaji.h"

namespace khiin::engine {
namespace {
namespace u8u = utf8::unchecked;
using namespace khiin::unicode;

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
    explicit BufferMgrImpl(Engine *engine) : m_engine(engine) {}

  private:
    //+---------------------------------------------------------------------------
    //
    // Protobuf-related methods
    //
    //----------------------------------------------------------------------------

    void BuildPreedit(proto::Preedit *preedit) override {
        {
            auto it = m_composition.CBegin();
            auto end = m_composition.CEnd();
            while (it != end) {
                if ((it != end - 1 && it->IsVirtualSpace() && !it[1].IsConverted()) || (!it->IsConverted())) {
                    auto composing_text = std::string();
                    while (it != end && (it->IsVirtualSpace() || !it->IsConverted())) {
                        composing_text += it->composed();
                        ++it;
                    }
                    auto *segment = preedit->add_segments();
                    segment->set_status(proto::SS_COMPOSING);
                    segment->set_value(composing_text);
                    continue;
                }

                if (it->IsVirtualSpace()) {
                    auto *segment = preedit->add_segments();
                    segment->set_value(" ");
                    segment->set_status(proto::SS_UNMARKED);
                } else if (it->IsConverted()) {
                    auto *segment = preedit->add_segments();
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

        preedit->set_caret(static_cast<int32_t>(m_caret));
        preedit->set_focused_caret(FocusedCaret());
    }

    void GetCandidates(proto::CandidateList *candidate_list) override {
        AdjustKhinAndSpacing(m_candidates);

        if (m_edit_state == EditState::Converted) {
            return;
        }

        auto id = 0;
        for (auto &cand : m_candidates) {
            auto *candidate_output = candidate_list->add_candidates();
            candidate_output->set_value(cand.Text());
            candidate_output->set_id(id);
            ++id;
        }

        candidate_list->set_focused(static_cast<int32_t>(m_focused_candidate));
    }

    proto::EditState edit_state() override {
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

    bool IsEmpty() override {
        return m_composition.Empty() && m_precomp.Empty() && m_postcomp.Empty();
    }

    void Clear() override {
        m_composition.Clear();
        m_candidates.clear();
        m_caret = 0;
        m_edit_state = EditState::Empty;
        m_nav_mode = NavMode::ByCharacter;
        m_focused_candidate = 0;
        m_focused_element = 0;
    }

    void Commit() override {
        if (IsEmpty()) {
            Clear();
            return;
        }

        m_engine->dictionary()->RecordNGrams(m_composition);
        Clear();
    }

    void Revert() override {
        switch (m_edit_state) {
        case EditState::Composing:
            if (m_composition.AllComposing()) {
                Clear();
            } else {
                m_composition.SetConverted(false);
                AdjustKhinAndSpacing(m_composition);
                SetCaretToEnd();
                m_focused_element = 0;
            }

            return;
        case EditState::Converted: {
            auto it = m_composition.Begin();
            safe_advance(it, m_composition.End(), m_focused_element);
            it->SetConverted(false);
            ++it;
            auto raw_caret = Buffer::RawTextSize(m_composition.Begin(), it);
            AdjustKhinAndSpacing(m_composition);
            SetCaretFromRaw(raw_caret);
            FocusElement(m_composition.CIterCaret(m_caret));
            m_edit_state = EditState::Composing;
            return;
        }
        case EditState::Selecting: {
            m_edit_state = EditState::Converted;
            return;
        }
        case EditState::Empty:
            return;
        }
    }

    void Insert(char ch) override {
        m_edit_state = EditState::Composing;

        switch (input_mode()) {
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

    void Erase(CursorDirection direction) override {
        if (direction == CursorDirection::L) {
            MoveCaret(CursorDirection::L);
        }

        auto elem_it = m_composition.IterCaret(m_caret);
        auto pos_in_elem = m_caret - Buffer::TextSize(m_composition.Begin(), elem_it);

        if (elem_it->size() == pos_in_elem) {
            ++elem_it;
            pos_in_elem = 0;
        }

        if (elem_it->IsConverted()) {
            EraseConverted(elem_it, pos_in_elem);
        } else {
            EraseComposing(direction);
        }
    }

    void HandleLeftRight(CursorDirection direction) override {
        if (m_edit_state == EditState::Composing) {
            MoveCaret(direction);
        } else if (m_edit_state == EditState::Converted) {
            MoveFocusOrCaret(direction);
        }
    }

    bool HandleSelectOrCommit() override {
        if (m_edit_state == EditState::Selecting) {
            SelectCandidate(m_focused_candidate);
            return false;
        }

        Commit();
        return true;
    }

    void HandleSelectOrFocus() override {
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

    void FocusNextCandidate() override {
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

    void FocusPrevCandidate() override {
        m_edit_state = EditState::Selecting;

        if (m_focused_candidate == 0) {
            FocusCandidate(m_candidates.size() - 1);
        } else {
            FocusCandidate(m_focused_candidate - 1);
        }
    }

    void FocusCandidate(size_t index) override {
        if (m_candidates.empty()) {
            m_edit_state = EditState::Converted;
            return;
        }

        assert(index < m_candidates.size());

        FocusCandidate_(index, false);
    }

    void SelectCandidate(size_t index) override {
        SelectCandidate_(index);
    }

    //+---------------------------------------------------------------------------
    //
    // BufferMgr private methods
    //
    //----------------------------------------------------------------------------
    //+----------------------------------
    // Buffer focus navigation
    //

    void BeginSegmentNavigation() {
        m_nav_mode = NavMode::BySegment;
        m_edit_state = EditState::Converted;
        SetCaretToEnd();
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

    void MoveCaret(CursorDirection direction) {
        auto buffer_text = m_composition.Text();
        SetCaret(Lomaji::MoveCaret(buffer_text, m_caret, direction));

        if (m_edit_state != EditState::Composing) {
            FocusElement(m_composition.CIterCaret(m_caret));
        }
    }

    void MoveFocus(CursorDirection direction) {
        auto it = m_composition.CBegin();
        safe_advance(it, m_composition.CEnd(), m_focused_element);

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

        FocusElement(it);
    }

    void FocusElement(Buffer::const_iterator element) {
        if (auto idx = static_cast<size_t>(std::distance(m_composition.CBegin(), element)); idx != m_focused_element) {
            OnFocusElementChange(idx);
        }
    }

    void OnFocusElementChange(size_t new_focused_element_idx) {
        assert(new_focused_element_idx < m_composition.Size());

        auto it = m_composition.Begin();
        auto end = m_composition.End();
        safe_advance(it, end, new_focused_element_idx);

        while (it != end && it != end - 1 && it->IsVirtualSpace()) {
            ++it;
        }

        SetFocusedElement(std::distance(m_composition.Begin(), it));

        if (m_edit_state == EditState::Converted) {
            UpdateCandidatesForFocusedElement();
        }
    }

    void UpdateCandidatesForFocusedElement() {
        if (m_composition.Empty()) {
            m_candidates.clear();
            return;
        }

        auto raw_text = m_composition.RawTextFrom(m_focused_element);
        m_candidates = CandidateFinder::MultiMatch(m_engine, FocusLGram(), raw_text);
        SetFocusedCandidateIndexToCurrent();
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

    void AdjustThenUpdateCaretAndFocus(size_t raw_caret, size_t focus_caret) {
        focus_caret = (std::min)(focus_caret, m_composition.RawTextSize());
        AdjustKhinAndSpacing(m_composition);
        FocusElement(m_composition.CIterRawCaret(focus_caret));
        SetCaretFromRaw(raw_caret);
    }

    void JoinBufferUpdateCaretAndFocus(utf8_size_t raw_caret) {
        // After joining, focus moves to the first element past m_precomp
        // Save this position as a virtual raw caret, since its actual
        // position may be affected by virtual spacing
        auto focus_raw_caret = m_composition.Empty() ? m_precomp.RawTextSize() : m_precomp.RawTextSize() + 1;

        m_composition.Join(&m_precomp, &m_postcomp);
        AdjustThenUpdateCaretAndFocus(raw_caret, focus_raw_caret);
    }

    std::pair<std::string, size_t> BeginInsertion(char ch) {
        SplitBufferForComposition();
        auto raw_composition = m_composition.RawText();
        auto raw_caret = m_composition.RawCaretFrom(m_caret);
        auto it = raw_composition.begin();
        u8u::advance(it, raw_caret);
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
        m_candidates = CandidateFinder::ContinuousMultiMatch(m_engine, FocusLGram(), raw_composition);
        m_composition = m_candidates[0];
        m_composition.SetConverted(false);
        assert(m_composition.RawText() == raw_composition);
    }

    void SetCompositionAndCandidatesBasic(std::string const &raw_composition) {
        m_candidates = CandidateFinder::MultiMatch(m_engine, FocusLGram(), raw_composition);

        if (!m_candidates.empty()) {
            m_composition = m_candidates[0];
            m_composition.SetConverted(false);

            auto raw_comp_size = u8_size(raw_composition);
            auto top_cand_size = m_composition.RawTextSize();

            if (raw_comp_size > top_cand_size) {
                m_composition.Append(raw_composition.substr(top_cand_size, raw_comp_size));
            }
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
    // Erasing
    //

    void EraseConverted(BufferElementList::iterator caret_element, utf8_size_t caret) {
        auto text = caret_element->converted();
        auto end = Lomaji::MoveCaret(text, caret, CursorDirection::R);
        auto from = text.begin();
        auto to = text.begin();
        u8u::advance(from, caret);
        u8u::advance(to, end);
        text.erase(from, to);

        if (text.empty()) {
            m_composition.Erase(caret_element);
        } else {
            caret_element->Replace(text);
        }

        m_composition.StripVirtualSpacing();

        if (m_composition.Empty()) {
            Clear();
            return;
        }

        EnsureCaretAndFocusInBounds();
        UpdateCandidatesForFocusedElement();
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
            m_composition.StripVirtualSpacing();
            EnsureCaretAndFocusInBounds();
        }

        if (IsEmpty()) {
            Clear();
            return;
        }

        switch (input_mode()) {
        case InputMode::Continuous:
            SetCompositionAndCandidatesContinuous(m_composition.RawText());
            break;
        case InputMode::Basic:
            SetCompositionAndCandidatesBasic(m_composition.RawText());
            break;
        case InputMode::Manual:
            break;
        }

        JoinBufferUpdateCaretAndFocus(raw_caret);
    }

    //+----------------------------------
    // Candidate navigation & selection
    //

    size_t FocusCandidate_(size_t index, bool selected) {
        auto raw_caret = m_composition.RawCaretFrom(m_caret);
        Buffer candidate = m_candidates.at(index);
        candidate.SetSelected(selected);
        auto adjusted_candidate_size = AdjustedSize(candidate);

        m_composition.SplitAtElement(m_focused_element, &m_precomp, nullptr);

        auto begin = m_composition.Begin();
        auto raw_candidate_size = candidate.RawTextSize();
        auto it = m_composition.IterRawCaret(raw_candidate_size);
        assert(it != m_composition.End());
        ++it;
        auto raw_buffer_size = Buffer::RawTextSize(begin, it);
        auto end = m_composition.End();

        assert(raw_buffer_size >= raw_candidate_size);

        if (raw_buffer_size > raw_candidate_size) {
            auto raw_text = Buffer::RawText(begin, it);
            auto deconverted = std::string(raw_text.begin() + static_cast<int>(raw_candidate_size), raw_text.end());

            while (it != end) {
                if (!CandidateFinder::HasExactMatch(m_engine, deconverted) || deconverted.empty()) {
                    deconverted.append(it->raw());
                    ++it;
                } else {
                    break;
                }
            }

            auto next_buf = CandidateFinder::ContinuousSingleMatch(m_engine, FocusLGram(), deconverted);

            if (next_buf.Empty()) {
                next_buf.Append(std::move(deconverted));
            }

            next_buf.SetConverted(false);
            candidate.Append(next_buf);

            auto cand_raw_size = candidate.RawTextSize();
            auto pre_raw_size = m_precomp.RawTextSize();
            raw_caret = (std::max)(raw_caret, cand_raw_size + pre_raw_size);
        }

        it = m_composition.Replace(begin, it, candidate);
        it += static_cast<int>(candidate.Size());
        end = m_composition.End();

        while (it != end && !it->IsSelected()) {
            it->SetConverted(false);
            ++it;
        }

        m_focused_candidate = index;

        JoinBufferUpdateCaretAndFocus(raw_caret);
        return adjusted_candidate_size;
    }

    void SelectCandidate_(size_t index) {
        assert(index < m_candidates.size());

        // Move the candidate into the composition
        auto candidate_size = FocusCandidate_(index, true);

        // When |EditState::Selecting|, buffer focus moves to the next element
        auto comp_begin = m_composition.Begin();
        auto focused_elem = comp_begin + static_cast<int>(m_focused_element);
        auto comp_end = m_composition.End();
        size_t n_elems_remaining = std::distance(focused_elem, comp_end);

        if (m_edit_state == EditState::Selecting && n_elems_remaining > candidate_size) {
            focused_elem += static_cast<int>(candidate_size);
        }

        // Don't let a virtual space become focused
        while (focused_elem != comp_end && focused_elem->IsVirtualSpace()) {
            ++focused_elem;
        }

        SetFocusedElement(std::distance(comp_begin, focused_elem));
        UpdateCandidatesForFocusedElement();
        BeginSegmentNavigation();
    }

    void SetFocusedCandidateIndexToCurrent() {
        auto const &current = m_composition.At(m_focused_element);

        for (size_t i = 0; i < m_candidates.size(); ++i) {
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
    // Helpers
    //

    size_t AdjustedSize(Buffer const &buffer) {
        auto copy = buffer;
        AdjustKhinAndSpacing(copy);
        return copy.Size();
    }

    void AdjustKhinAndSpacing(Buffer &buffer) {
        KhinHandler::AutokhinBuffer(m_engine, buffer);
        buffer.AdjustVirtualSpacing();
    }

    void AdjustKhinAndSpacing(std::vector<Buffer> &buffers) {
        for (auto &buffer : buffers) {
            AdjustKhinAndSpacing(buffer);
        }
    }

    void EnsureCaretAndFocusInBounds() {
        SetCaret(m_caret);
        SetFocusedElement(m_focused_element);
    }

    void SetFocusedElement(size_t index) {
        auto comp_size = m_composition.Size();
        if (comp_size == 0) {
            m_focused_element = 0;
        } else {
            m_focused_element = (std::min)(index, comp_size - 1);
        }
    }

    void SetCaret(size_t caret) {
        m_caret = (std::min)(caret, m_composition.TextSize());
    }

    void SetCaretFromRaw(size_t raw_caret) {
        SetCaret(m_composition.CaretFrom(raw_caret));
    }

    void SetCaretToEnd() {
        m_caret = m_composition.TextSize();
    }

    // void SetCaretToElementEnd() {
    //    if (m_focused_element == m_composition.Size() - 1) {
    //        SetCaretToEnd();
    //    }
    //}

    TaiToken *FocusLGram() {
        if (!m_precomp.Empty() && m_focused_element == m_precomp.Size()) {
            auto it = m_precomp.End() - 1;

            if (it->IsVirtualSpace() && it != m_precomp.Begin()) {
                --it;
            }

            if (it->IsTaiText()) {
                return it->candidate();
            }
        } else if (m_focused_element > 0) {
            auto it = m_composition.Begin();
            auto end = m_composition.End();
            safe_advance(it, end, m_focused_element - 1);

            if (it == end) {
                return nullptr;
            }

            // auto it = m_composition.Begin() + static_cast<int>(m_focused_element) - 1;

            if (it->IsVirtualSpace()) {
                if (it == m_composition.Begin()) {
                    if (!m_precomp.Empty() && m_precomp.Back().IsTaiText()) {
                        return m_precomp.Back().candidate();
                    }
                } else {
                    --it;
                }
            }

            if (it->IsTaiText()) {
                return it->candidate();
            }
        }

        return nullptr;
    }

    int FocusedCaret() {
        if (m_focused_element == 0) {
            return 0;
        }

        auto begin = m_composition.CBegin();
        auto end = m_composition.CBegin();
        safe_advance(end, m_composition.CEnd(), m_focused_element - 1);

        while (end != begin && end->IsVirtualSpace()) {
            --end;
        }

        auto last_elem_size = end->size();
        return static_cast<int>(Buffer::TextSize(begin, end) + last_elem_size);
    }

    InputMode input_mode() {
        return m_engine->config()->input_mode();
    }

    // SyllableParser *parser() {
    //    return m_engine->syllable_parser();
    //}

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

BufferMgr::~BufferMgr() = default;

std::unique_ptr<BufferMgr> BufferMgr::Create(Engine *engine) {
    return std::make_unique<BufferMgrImpl>(engine);
}

} // namespace khiin::engine

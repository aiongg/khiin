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

BufferElementList ConvertWholeBuffer(BufferElementList elements) {
    for (auto &elem : elements) {
        elem.is_converted = true;
    }
    Buffer::AdjustVirtualSpacing(elements);
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
        // if (!HasComposingSection(m_composition)) {
        // MoveFocus(direction);
        //} else {
        //}
    }

    virtual void Erase(CursorDirection direction) override {
        if (direction == CursorDirection::L) {
            MoveCaret(CursorDirection::L);
        }

        auto elem_it = m_composition.At(m_caret);
        auto caret = m_caret - Buffer::Size(m_composition.Begin(), elem_it);

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

    virtual void FocusCandidate(int index) override {}

    virtual void SetInputMode(InputMode new_mode) override {
        m_input_mode = new_mode;
    }

    virtual void SelectNextCandidate() override {
        if (m_input_mode == InputMode::Continuous && m_edit_state == EditState::EDIT_COMPOSING) {
            m_edit_state = EditState::EDIT_CONVERTED;
            m_nav_mode = NavMode::BySegment;
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

            m_focused_element = std::distance(m_composition.Begin(), it);
        } else if (m_nav_mode == NavMode::ByCharacter) {
            MoveCaretBasic(direction);
            auto it = m_composition.At(m_caret);
            m_focused_element = std::distance(m_composition.Begin(), it);
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
        Segmenter::SegmentText(m_engine, raw_composition, m_composition.get());
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), m_composition.get());

        UpdateCandidatesContinuous(raw_composition);
        raw_caret = m_composition.Join(raw_caret, m_precomp, m_postcomp);
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
            m_caret -= m_precomp.Size();
        }
    }

    void UpdateCandidatesContinuous(std::string const &raw_buffer) {
        m_candidates.clear();

        //if (m_input_mode == InputMode::Continuous && m_edit_state == EditState::EDIT_COMPOSING) {
        //    m_candidates.push_back(ConvertWholeBuffer(m_composition.get()));
        //}

        auto candidates = CandidateFinder::ContinuousCandidates(m_engine, nullptr, raw_buffer);
        // auto candidates = CandidateFinder::GetCandidatesFromStart(m_engine, nullptr, raw_buffer);

        for (auto &c : candidates) {
            //if (!m_candidates.empty() && ContainsCandidate(m_candidates[0], c)) {
            //    continue;
            //}

            m_candidates.push_back(BufferElementList(c.get()));
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

    void SelectCandidate(size_t index) {
        if (index >= m_candidates.size()) {
            return;
        }

        if (m_composition.HasComposing()) {
            m_composition.IsolateComposing(m_precomp, m_postcomp);

            if (!m_precomp.Empty()) {
                m_caret -= m_precomp.Size();
            }
        }

        auto raw_caret = m_composition.RawCaretFrom(m_caret, m_engine->syllable_parser());
        auto &candidate = m_candidates.at(index);
        auto raw_buffer = GetRawBuffer();
        auto candidate_raw = candidate.RawText();
        auto raw_remainder = raw_buffer.substr(candidate_raw.size(), raw_buffer.size());

        m_composition.Clear();
        Segmenter::SegmentText(m_engine, raw_remainder, m_composition.get());
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), m_composition.get());
        m_composition.get().insert(m_composition.Begin(), candidate.Begin(), candidate.End());

        raw_caret = m_composition.Join(raw_caret, m_precomp, m_postcomp);
        m_caret = m_composition.CaretFrom(raw_caret, m_engine->syllable_parser());
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

    // enum class FocusedElementState {
    //    Focused,
    //    Composing,
    //};

    enum class NavMode {
        ByCharacter,
        BySegment,
    };

    Engine *m_engine = nullptr;
    utf8_size_t m_caret = 0;
    Buffer m_precomp;     // Converted elements before the composition
    Buffer m_composition; // Elements in the composition
    Buffer m_postcomp;    // Converted elements after the composition
    std::vector<Buffer> m_candidates;
    size_t m_focused_candidate = 0;
    int m_focused_element = 0;
    EditState m_edit_state = EditState::EDIT_EMPTY;
    InputMode m_input_mode = InputMode::Continuous;
    NavMode m_nav_mode = NavMode::ByCharacter;
    // FocusedElementState m_focused_element_state = FocusedElementState::Composing;
};

} // namespace

BufferMgr *BufferMgr::Create(Engine *engine) {
    return new BufferMgrImpl(engine);
}

} // namespace khiin::engine

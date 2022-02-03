#include "BufferMgr.h"

#include "BufferElement.h"
#include "Engine.h"
#include "Lomaji.h"
#include "Segmenter.h"
#include "unicode_utils.h"

namespace khiin::engine {

using namespace messages;

namespace {

class BufferMgrImpl : public BufferMgr {
  public:
    BufferMgrImpl(Engine *engine) : m_engine(engine) {}

    virtual void Clear() override {
        m_elements.clear();
        m_caret = 0;
    }

    virtual bool IsEmpty() override {
        return m_elements.empty();
    }

    virtual void Insert(char ch) override {
        switch (m_input_mode) {
        case InputMode::Continuous:
            InsertContinuous(ch);
            break;
        default:
            break;
        }
    }

    virtual void MoveCaret(CursorDirection direction) override {
        auto buffer_text = GetDisplayBuffer();
        m_caret = Lomaji::MoveCaret(buffer_text, m_caret, direction);
    }

    virtual void Erase(CursorDirection direction) override {
        if (direction == CursorDirection::L) {
            MoveCaret(CursorDirection::L);
        }

        auto raw_caret = GetRawCaret();
        auto remainder = m_caret;

        auto it = m_elements.begin();
        for (; it != m_elements.end(); ++it) {
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
            m_elements.erase(it);
        }
        auto raw_buffer = GetRawBuffer();
        UpdateBuffer(raw_buffer, raw_caret);
    }

    virtual void BuildPreedit(Preedit *preedit) override {
        auto segment = preedit->add_segments();
        segment->set_value(GetDisplayBuffer());
        segment->set_status(SegmentStatus::COMPOSING);
        preedit->set_cursor_position(m_caret);
    }

    virtual void MoveFocus(CursorDirection direction) override {}

    virtual void GetCandidates(messages::CandidateList *candidate_list) override {
        if (m_input_mode == InputMode::Continuous) {
            auto first_candidate = std::string();
            for (auto &elem : m_elements) {
                first_candidate += elem.converted();
            }
            auto cand = candidate_list->add_candidates();
            cand->set_value(first_candidate);
        }
    }

    virtual void FocusCandidate(int index) override {}

    virtual void SetInputMode(InputMode new_mode) override {
        m_input_mode = new_mode;
    }

  private:
    enum class FocusedElementState {
        Focused,
        Composing,
    };

    void InsertContinuous(char ch) {
        // Get raw buffer and cursor position
        auto raw_buffer = GetRawBuffer();
        auto raw_caret = GetRawCaret();
        raw_buffer.insert(raw_caret, 1, ch);
        ++raw_caret;
        UpdateBuffer(raw_buffer, raw_caret);
    }

    void UpdateBuffer(std::string const &raw_buffer, size_t raw_caret) {
        std::vector<BufferElement> elements;
        utf8_size_t new_caret_position = 0;
        m_engine->segmenter()->SegmentWholeBuffer(raw_buffer, raw_caret, elements, new_caret_position);
        m_elements = elements;
        UpdateCaret(raw_caret);
    }

    std::string GetRawBuffer() {
        auto ret = std::string();
        for (auto &elem : m_elements) {
            ret.append(elem.raw());
        }
        return ret;
    }

    std::string GetDisplayBuffer() {
        auto ret = std::string();
        for (auto &elem : m_elements) {
            ret.append(elem.composed());
        }
        return ret;
    }

    utf8_size_t GetDisplayBufferSize() {
        utf8_size_t ret = 0;
        for (auto &elem : m_elements) {
            ret += elem.size();
        }
        return ret;
    }

    size_t GetRawCaret() {
        if (m_elements.empty()) {
            return 0;
        }

        size_t raw_caret = 0;
        auto remainder = m_caret;

        auto it = m_elements.cbegin();
        for (; it != m_elements.cend(); ++it) {
            if (auto size = it->size(); remainder > size) {
                remainder -= size;
                raw_caret += it->raw().size();
            } else {
                break;
            }
        }

        raw_caret += it->ComposedToRawCaret(m_engine->syllable_parser(), remainder);
        return raw_caret;
    }

    void UpdateCaret(size_t raw_caret) {
        if (m_elements.empty()) {
            return;
        }

        utf8_size_t new_caret = 0;
        auto remainder = raw_caret;

        auto it = m_elements.cbegin();
        for (; it != m_elements.cend(); ++it) {
            auto raw = it->raw();
            if (auto raw_size = raw.size(); remainder > raw_size) {
                new_caret += it->size();
                remainder -= raw_size;
            } else {
                break;
            }
        }

        new_caret += it->RawToComposedCaret(m_engine->syllable_parser(), remainder);
        m_caret = new_caret;
    }

    Engine *m_engine = nullptr;
    utf8_size_t m_caret = 0;
    std::vector<BufferElement> m_elements;
    bool m_converted = false;
    int m_focused_element = 0;
    FocusedElementState m_focused_element_state = FocusedElementState::Composing;
    InputMode m_input_mode = InputMode::Continuous;
};

} // namespace

BufferMgr *BufferMgr::Create(Engine *engine) {
    return new BufferMgrImpl(engine);
}

} // namespace khiin::engine

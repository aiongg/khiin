#include "BufferMgr.h"

#include "BufferElement.h"
#include "Engine.h"
#include "Segmenter.h"
#include "unicode_utils.h"

namespace khiin::engine {

namespace {

class BufferMgrImpl : public BufferMgr {
  public:
    BufferMgrImpl(Engine *engine) : m_engine(engine) {}

    virtual void Clear() override {
        m_elements.clear();
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

    virtual void MoveCaret(CursorDirection direction) override {}

    virtual void Erase(CursorDirection direction) override {}

    virtual void BuildPreedit(messages::Preedit *preedit) override {
        for (auto &elem : m_elements) {
            auto segment = preedit->add_segments();
            segment->set_value(elem.composed());
        }
        preedit->set_cursor_position(m_caret);
    }

    virtual void MoveFocus(CursorDirection direction) override {}

    virtual void GetCandidates(messages::CandidateList *candidate_list) override {}

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

        std::vector<BufferElement> elements;
        utf8_size_t new_caret_position = 0;
        m_engine->segmenter()->SegmentWholeBuffer(raw_buffer, raw_caret, elements, new_caret_position);
        m_elements = elements;
        UpdateCaret(raw_caret);
    }

    std::string GetRawBuffer() {
        auto ret = std::string();
        for (auto i = 0; i < m_elements.size(); ++i) {
            auto &elem = m_elements[i];
            ret.append(elem.raw());
        }
        return ret;
    }

    size_t GetRawCaret() {
        size_t raw_caret = 0;
        auto remainder = m_caret;
        BufferElement *elem = nullptr;
        for (auto i = 0; i < m_elements.size(); ++i) {
            elem = &m_elements[i];
            if (auto size = elem->size(); remainder > size) {
                auto raw = elem->raw();
                remainder -= size;
                raw_caret += raw.size();
                continue;
            }

            break;
        }
        if (!elem) {
            return raw_caret;
        }

        raw_caret += elem->ComposedToRawCaret(m_engine->syllable_parser(), remainder);
        return raw_caret;
    }

    void UpdateCaret(size_t raw_caret) {
        utf8_size_t new_caret = 0;
        auto remainder = raw_caret;
        BufferElement *elem = nullptr;
        for (auto i = 0; i < m_elements.size(); ++i) {
            elem = &m_elements[i];
            auto raw = elem->raw();
            if (auto size = raw.size();  remainder > size) {
                new_caret += elem->size();
                remainder -= size;
                continue;
            }
            break;
        }

        if (!elem) {
            return;
        }

        new_caret += elem->RawToComposedCaret(m_engine->syllable_parser(), remainder);
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

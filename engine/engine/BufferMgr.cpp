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

    virtual void BuildPreedit(messages::Preedit *preedit) override {}

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
        std::string raw_buffer;
        size_t raw_caret = 0;
        GetRawBuffer(raw_buffer, raw_caret);
        raw_buffer.insert(raw_caret, 1, ch);
        ++raw_caret;

        std::vector<BufferElement> elements;
        utf8_size_t new_caret_position = 0;
        m_engine->segmenter()->SegmentWholeBuffer(raw_buffer, raw_caret, elements, new_caret_position);
    }

    void GetRawBuffer(std::string &raw_buffer, size_t &raw_caret) {
        raw_buffer.clear();

        size_t caret_elem_idx = 0;
        size_t caret_idx = 0;
        LocateCaret(caret_elem_idx, caret_idx);

        for (auto i = 0; i < m_elements.size(); ++i) {
            auto &elem = m_elements[i];

            if (i != caret_elem_idx) {
                auto raw = elem.Raw();
                raw_buffer.append(raw);
                raw_caret += raw.size();
                continue;
            }

            auto raw = std::string();
            size_t raw_caret = 0;
            elem.RawIndexed(caret_idx, raw, raw_caret);
            raw_caret += raw_caret;
        }
    }

    void LocateCaret(size_t &element_index, size_t &caret_index) {
        auto remainder = m_caret_position;

        for (auto i = 0; i < m_elements.size(); ++i) {
            auto &elem = m_elements[i];
            auto size = elem.Size();

            if (remainder > size) {
                remainder -= size;
                continue;
            } else {
                element_index = i;
                caret_index = remainder;
                return;
            }
        }
    }

    Engine *m_engine = nullptr;
    int m_caret_position = 0;
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

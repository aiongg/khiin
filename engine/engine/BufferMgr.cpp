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

using namespace messages;

namespace {

void AdjustVirtualSpacing(BufferElementList &elements) {
    using namespace unicode;

    if (elements.empty()) {
        return;
    }

    elements.erase(std::remove_if(elements.begin(), elements.end(),
                                  [](BufferElement const &el) {
                                      return el.IsVirtualSpace();
                                  }),
                   elements.end());

    for (auto i = elements.size() - 1; i != 0; --i) {
        std::string lhs =
            elements.at(i - 1).is_converted ? elements.at(i - 1).converted() : elements.at(i - 1).composed();
        std::string rhs = elements.at(i).is_converted ? elements.at(i).converted() : elements.at(i).composed();
        auto end_cat = end_glyph_type(lhs);
        auto start_cat = start_glyph_type(rhs);

        if (end_cat == GlyphCategory::Alnum && start_cat == GlyphCategory::Alnum ||
            end_cat == GlyphCategory::Alnum && start_cat == GlyphCategory::Khin ||
            end_cat == GlyphCategory::Alnum && start_cat == GlyphCategory::Hanji ||
            end_cat == GlyphCategory::Hanji && start_cat == GlyphCategory::Alnum) {
            elements.insert(elements.begin() + i, BufferElement(Spacer::VirtualSpace));
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

class BufferMgrImpl : public BufferMgr {
  public:
    BufferMgrImpl(Engine *engine) : m_engine(engine) {}

    virtual void Clear() override {
        m_elements.clear();
        m_candidates.clear();
        m_caret = 0;
        m_buffer_state = BufferState::Empty;
    }

    virtual bool IsEmpty() override {
        return m_elements.empty();
    }

    virtual void Insert(char ch) override {
        if (m_buffer_state == BufferState::Empty) {
            m_buffer_state = BufferState::InitialComposition;
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
        if (m_buffer_state == BufferState::InitialComposition) {
            auto segment = preedit->add_segments();
            segment->set_value(GetDisplayBuffer());
            segment->set_status(SegmentStatus::COMPOSING);
        } else if (m_buffer_state == BufferState::Converted) {
            auto i = 0;
            for (auto &elem : m_elements) {
                auto segment = preedit->add_segments();
                if (elem.is_converted) {
                    segment->set_value(elem.converted());
                } else {
                    segment->set_value(elem.composed());
                }
                if (!elem.IsVirtualSpace()) {
                    if (i == m_focused_element) {
                        segment->set_status(SegmentStatus::FOCUSED);
                    } else {
                        segment->set_status(SegmentStatus::CONVERTED);
                    }
                }
                ++i;
            }
        }

        preedit->set_cursor_position(static_cast<int>(m_caret));
    }

    virtual void GetCandidates(messages::CandidateList *candidate_list) override {
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
        }
    }

    virtual void MoveFocus(CursorDirection direction) override {}

    virtual void FocusCandidate(int index) override {}

    virtual void SetInputMode(InputMode new_mode) override {
        m_input_mode = new_mode;
    }

    virtual void SelectNextCandidate() override {
        //if (m_input_mode == InputMode::Continuous && m_buffer_state == BufferState::InitialComposition) {
            //ConvertWholeBuffer();
        //}
    }

  private:
    enum class BufferState {
        Empty,
        InitialComposition,
        Converted,
    };

    BufferState m_buffer_state = BufferState::Empty;

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
        UpdateCandidates(raw_buffer);
    }

    void UpdateBuffer(std::string const &raw_buffer, size_t raw_caret) {
        std::vector<BufferElement> elements;
        Segmenter::SegmentWholeBuffer(m_engine, raw_buffer, elements);
        KhinHandler::AutokhinBuffer(m_engine->syllable_parser(), elements);
        AdjustVirtualSpacing(elements);
        m_elements = elements;
        UpdateCaret(raw_caret);
    }

    void UpdateCandidates(std::string const &raw_buffer) {
        m_candidates.clear();

        if (m_input_mode == InputMode::Continuous && m_buffer_state == BufferState::InitialComposition) {
            m_candidates.push_back(ConvertWholeBuffer(m_elements));
        }

        auto candidates = CandidateFinder::GetCandidatesFromStart(m_engine, nullptr, raw_buffer);

        for (auto &c : candidates) {
            m_candidates.push_back(std::move(c));
        }
    }

    std::string GetRawBuffer() {
        return BufferElement::raw(m_elements);
    }

    std::string GetDisplayBuffer() {
        auto ret = std::string();
        for (auto &elem : m_elements) {
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
    BufferElementList m_elements; // DISPLAY BUFFER ONLY
    std::vector<BufferElementList> m_candidates;
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

#pragma once

#include <iterator>
#include <string>
#include <vector>

#include "CandidateFinder.h"
#include "errors.h"

namespace khiin::engine {

// Foreward declarations
struct Segment;
class SynchronizedBuffer;
using Segments = std::vector<Segment>;

struct Caret {
    SynchronizedBuffer *buf = nullptr;
    size_t segment_pos = size_t(0);
    size_t input_pos = size_t(0);
    size_t display_pos = size_t(0);

    void operator++();
    void operator++(int);
    void operator--();
    void operator--(int);

    bool IsAtEnd();
    void ShiftToStart();
    void ShiftToEnd();

    auto displayOffset() -> size_t;
    auto displayOffset(Segments::iterator from) -> size_t;
    auto rawOffset(Segments::iterator from) -> size_t;
    auto setBuffer(SynchronizedBuffer *pBuf) {
        buf = pBuf;
    };
    auto syncToRaw(size_t segmentStart, size_t rawOffset) -> void;
};

struct Segment {
    std::string input_value = std::string();
    std::string display_value = std::string();
    const Token *token = nullptr;
    bool editing = true;
    bool converted = false;
    bool focused = false;
    bool spaced = false;
};

class SynchronizedBuffer {
    friend struct Caret;

  public:
    SynchronizedBuffer() {
        m_caret.setBuffer(this);
        m_segments.reserve(100);
        m_segments.push_back(Segment());
    };

    auto Reset() -> void;
    auto displayCursorOffset() -> size_t;
    auto displayText() -> std::string;

    Segments::iterator begin();
    Segments::iterator end();
    Segments::iterator edit_begin();
    Segments::iterator edit_end();
    Segments::iterator caret();
    std::string::iterator input_caret();
    std::string::iterator display_caret();

    Segments::const_iterator cbegin();
    Segments::const_iterator cend();
    Segments::const_iterator edit_cbegin();
    Segments::const_iterator edit_cend();
    Segments::const_iterator ccaret();
    std::string::const_iterator input_ccaret();
    std::string::const_iterator display_ccaret();

    auto empty() -> bool;
    auto erase(CursorDirection dir) -> void;
    auto insert(char ch) -> void;
    auto isCursorAtEnd() -> bool;
    auto MoveCaret(CursorDirection dir) -> void;
    auto moveCursorToEnd() -> void;
    auto rawText(Segments::iterator first, Segments::iterator last) -> std::string;
    auto segmentCount() -> size_t;
    auto segmentByCandidate(Segments::iterator first, Segments::iterator last, const Candidate &candidates) -> void;
    auto updateSegmentSpacing() -> void;

  private:
    auto erase(size_t len) -> void;
    auto eraseSegment(Segments::iterator first) -> void;
    auto removeToneFromRawBuffer() -> void;

    Segments m_segments;
    Caret m_caret;
    int m_focused_segment = -1;
};

} // namespace khiin::engine

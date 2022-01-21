#pragma once

#include <iterator>
#include <string>
#include <vector>

#include "candidates.h"
#include "errors.h"

namespace taikey {

// Foreward declarations
struct Segment;
class SynchronizedBuffer;
using Segments = std::vector<Segment>;
using SegmentIter = Segments::iterator;
using SegmentCIter = Segments::const_iterator;

enum class CursorDirection {
    L,
    R,
};

struct Cursor {
    SynchronizedBuffer *buf = nullptr;
    size_t segment = size_t(0);
    size_t raw = size_t(0);
    size_t display = size_t(0);

    void operator++();
    void operator++(int);
    void operator--();
    void operator--(int);

    auto atEnd() -> bool;
    auto clear() -> void;
    auto displayOffset() -> size_t;
    auto displayOffset(SegmentIter from) -> size_t;
    auto rawOffset(SegmentIter from) -> size_t;
    auto setBuffer(SynchronizedBuffer *bufp) { buf = bufp; };
    auto setToEnd() -> void;
    auto syncToRaw(size_t segmentStart, size_t rawOffset) -> void;
};

struct Segment {
    std::string raw = std::string();
    std::string display = std::string();
    const Token *token = nullptr;
    //Candidate candidates_;
    //size_t selectedCandidate = size_t(0);
    bool selected = false;
    bool confirmed = false;
    bool editing = true;
    bool spaced = false;
};

class SynchronizedBuffer {
    friend struct Cursor;

  public:
    SynchronizedBuffer() {
        cursor.setBuffer(this);
        segments.reserve(100);
        segments.push_back(Segment());
    };

    auto clear() -> void;
    auto displayCursor() -> std::string::iterator;
    auto displayCursorOffset() -> size_t;
    auto displayText() -> std::string;
    auto editingBegin() -> SegmentIter;
    auto editingEnd() -> SegmentIter;
    auto empty() -> bool;
    auto erase(CursorDirection dir) -> void;
    auto insert(char ch) -> void;
    auto isCursorAtEnd() -> bool;
    auto moveCursor(CursorDirection dir) -> void;
    auto moveCursorToEnd() -> void;
    auto rawCursor() -> std::string::iterator;
    auto rawText(SegmentIter first, SegmentIter last) -> std::string;
    auto segmentAtCursor() -> SegmentIter;
    auto segmentBegin() -> SegmentIter;
    auto segmentCount() -> size_t;
    auto segmentEnd() -> SegmentIter;
    auto segmentByCandidate(SegmentIter first, SegmentIter last, const Candidate &candidates)
        -> void;
    auto updateSegmentSpacing() -> void;

  private:
    auto erase(size_t len) -> void;
    auto eraseSegment(SegmentIter first) -> void;
    auto removeToneFromRawBuffer() -> void;

    Segments segments;
    Cursor cursor;
    // size_t focusedSegment;
};

} // namespace TaiKey
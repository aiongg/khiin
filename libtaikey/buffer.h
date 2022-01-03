#pragma once

#include <iterator>
#include <string>
#include <vector>

#include "candidates.h"
#include "errors.h"

namespace TaiKey {

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
    Candidates candidates;
    size_t selectedCandidate = size_t(0);
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

    auto candidateAt(SegmentIter it) -> const Candidate &;
    auto clear() -> void;
    auto displayCursor() -> std::string::iterator;
    auto displayCursorOffset() -> size_t;
    auto displayText() -> std::string;
    auto editingBegin() -> SegmentIter;
    auto erase(CursorDirection dir) -> void;
    auto insert(char ch) -> void;
    auto isCursorAtEnd() -> bool;
    auto moveCursor(CursorDirection dir) -> void;
    auto rawCursor() -> std::string::iterator;
    auto rawText(SegmentIter first, SegmentIter last) -> std::string;
    auto segmentAtCursor() -> SegmentIter;
    auto segmentBegin() -> SegmentIter;
    auto segmentCount() -> size_t;
    auto segmentEnd() -> SegmentIter;
    auto setPrimaryCandidate(SegmentIter from, Candidates nextPrimaryCandidate)
        -> void;

  private:
    auto erase(size_t len) -> void;
    auto eraseSegment(SegmentIter first) -> void;
    auto removeToneFromRawBuffer() -> void;
    auto updateSegmentSpacing() -> void;

    Candidates primaryCandidate;
    Segments segments;
    Cursor cursor;
};

} // namespace TaiKey
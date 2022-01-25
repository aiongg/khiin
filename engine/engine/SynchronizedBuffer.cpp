#include <algorithm>

#include <c9/zip.h>
#include <utf8cpp/utf8.h>

#include "SynchronizedBuffer.h"
#include "Lomaji.h"

namespace khiin::engine {

// Utility

auto displayFromRaw(std::string raw, std::string split) {
    if (split.empty()) {
        return asciiSyllableToUtf8(raw);
    }

    auto display = std::string();
    auto syls = spaceAsciiByUtf8(raw, split);
    auto autokhin = false;

    for (auto it = syls.begin(); it != syls.end(); ++it) {
        auto &s = *it;
        if (s.size() > 2 && s[0] == '-' && s[1] == '-') {
            autokhin = true;
        }

        if (autokhin && s[0] != '-') {
            s.insert(0, 2, '-');
        }

        display += asciiSyllableToUtf8(s);
        if (it != syls.end() - 1 && display.back() != '-') {
            display += ' ';
        }
    }

    return display;
}

// SynchronizedBuffer

auto SynchronizedBuffer::clear() -> void {
    cursor.clear();
    // primaryCandidate.clear();
    segments.clear();
    segments.push_back(Segment());
}

auto SynchronizedBuffer::displayCursorOffset() -> size_t {
    return cursor.displayOffset();
}

auto SynchronizedBuffer::displayCursor() -> std::string::iterator {
    auto s = segmentAtCursor();
    auto it = (*s).display.begin();
    utf8::advance(it, cursor.display, (*s).display.end());
    return it;
}

auto SynchronizedBuffer::displayText() -> std::string {
    auto ret = std::string();

    for (auto &s : segments) {
        ret += s.display;
        if (s.spaced) {
            ret += ' ';
        }
    }

    return ret;
}

auto SynchronizedBuffer::editingBegin() -> SegmentIter {
    for (auto it = segments.begin(); it != segments.end(); it++) {
        if ((*it).editing) {
            return it;
        }
    }

    return segments.end();
}

auto SynchronizedBuffer::editingEnd() -> SegmentIter {
    for (auto it = segments.end() - 1; it != segments.begin(); --it) {
        if ((*it).editing) {
            return ++it;
        }
    }

    return segments.end();
}

auto SynchronizedBuffer::empty() -> bool {
    return segments.size() == 1 && segments[0].raw.size() == 0;
}

auto SynchronizedBuffer::erase(CursorDirection dir) -> void {
    if (dir == CursorDirection::L) {
        if (cursor.raw == 0) {
            cursor--;
            return;
        }

        cursor--;
    }

    erase(1);
}

auto SynchronizedBuffer::insert(char ch) -> void {
    auto it = segmentAtCursor();
    auto &s = *it;
    s.raw.insert(cursor.raw, 1, ch);
    ++(cursor.raw);
}

auto SynchronizedBuffer::isCursorAtEnd() -> bool { return cursor.atEnd(); };

auto SynchronizedBuffer::moveCursor(CursorDirection dir) -> void {
    if (dir == CursorDirection::R) {
        cursor++;
    } else if (dir == CursorDirection::L) {
        cursor--;
    }
}

auto SynchronizedBuffer::moveCursorToEnd() -> void { cursor.setToEnd(); }

auto SynchronizedBuffer::rawCursor() -> std::string::iterator {
    auto s = segmentAtCursor();
    auto it = (*s).raw.begin() + cursor.raw;
    return it;
}

auto SynchronizedBuffer::rawText(SegmentIter first, SegmentIter last)
    -> std::string {
    std::string ret;
    ret.reserve(100);

    while (first != last) {
        auto &s = *(first);
        ret.append(s.raw);
        ++first;
    }

    return std::move(ret);
}

auto SynchronizedBuffer::segmentCount() -> size_t { return segments.size(); }

auto SynchronizedBuffer::segmentBegin() -> SegmentIter {
    return segments.begin();
}

auto SynchronizedBuffer::segmentEnd() -> SegmentIter { return segments.end(); }

auto SynchronizedBuffer::segmentAtCursor() -> SegmentIter {
    return segments.begin() + cursor.segment;
}

auto SynchronizedBuffer::segmentByCandidate(SegmentIter first, SegmentIter last,
                                            const Candidate &candidate)
    -> void {
    // Save for afterwards
    auto atEnd = cursor.atEnd();
    auto rawCursorOffset = cursor.rawOffset(first);
    auto segmentIdx = std::distance(segments.begin(), first);

    // Build new segments & re-attach
    auto nextSegments = Segments();
    for (auto &chunk : candidate) {
        nextSegments.push_back(
            Segment{chunk.raw, displayFromRaw(chunk.raw, chunk.token.input),
                    &chunk.token});
    }
    first = segments.erase(first, last);
    segments.insert(first, std::make_move_iterator(nextSegments.cbegin()),
                    std::make_move_iterator(nextSegments.cend()));

    nextSegments.clear();

    // Update cursor
    // Case: at the end - stay at the end
    if (atEnd) {
        cursor.setToEnd();
    } else {
        cursor.syncToRaw(segmentIdx, rawCursorOffset);
    }

    updateSegmentSpacing();
}

auto SynchronizedBuffer::updateSegmentSpacing() -> void {
    if (segments.size() < 2) {
        return;
    }

    auto tokens = std::vector<std::string_view>();
    std::transform(
        segments.cbegin(), segments.cend(), std::back_inserter(tokens),
        [](const Segment &s) { return std::string_view(s.display); });

    auto spaces = tokenSpacer(tokens);

    for (auto &&[spaced, segment] : c9::zip(spaces, segments)) {
        segment.spaced = spaced;
    }
}

// Private

auto SynchronizedBuffer::erase(size_t len) -> void {
    auto it = segmentAtCursor();
    auto &s = *it;

    auto r_start = rawCursor();
    auto d_start = displayCursor();

    auto r_it = std::string::iterator(r_start);
    auto d_it = std::string::iterator(d_start);

    parallelNext(r_it, s.raw.end(), d_it, s.display.end());

    if (hasToneDiacritic(std::string(d_start, d_it))) {
        removeToneFromRawBuffer();
    }

    s.raw.erase(r_start, r_it);
    if (s.raw.empty()) {
        eraseSegment(it);
    }

    if (segments.size() == 0) {
        segments.push_back(Segment());
    }
}

auto SynchronizedBuffer::eraseSegment(SegmentIter first) -> void {
    auto &s = *first;

    if (segmentAtCursor() == first) {
        cursor--;
    }

    segments.erase(first);
    updateSegmentSpacing();
}

auto SynchronizedBuffer::removeToneFromRawBuffer() -> void {
    auto s_it = segmentAtCursor();
    auto &s = *s_it;

    auto r_it = s.raw.begin() + cursor.raw;

    static auto isSyllableBreak = [](char ch) {
        return ch == ' ' || ch == '-' || ch == '0';
    };

    // First search to the right
    while (r_it != s.raw.end() && !isSyllableBreak(*r_it)) {
        if (isdigit(*r_it)) {
            s.raw.erase(r_it, r_it + 1);
            return;
        }
        ++r_it;
    }

    --r_it;

    // Then search to the left (and update cursor position)
    while (r_it != s.raw.begin() && !isSyllableBreak(*r_it)) {
        if (isdigit(*r_it)) {
            s.raw.erase(r_it, r_it + 1);
            cursor.raw--;
        }
    }
}

} // namespace khiin::engine
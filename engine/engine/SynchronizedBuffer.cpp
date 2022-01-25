#include <algorithm>

#include <c9/zip.h>
#include <utf8cpp/utf8.h>

#include "Lomaji.h"
#include "SynchronizedBuffer.h"
#include "ParallelIterators.h"

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

//+---------------------------------------------------------------------------
//
// SynchronizedBuffer
//
//----------------------------------------------------------------------------

auto SynchronizedBuffer::Reset() -> void {
    m_caret.ShiftToStart();
    m_segments.clear();
    m_segments.push_back(Segment());
}

auto SynchronizedBuffer::displayCursorOffset() -> size_t {
    return m_caret.displayOffset();
}

auto SynchronizedBuffer::displayText() -> std::string {
    auto ret = std::string();

    for (auto &s : m_segments) {
        ret += s.display_value;
        if (s.spaced) {
            ret += ' ';
        }
    }

    return ret;
}

//+---------------------------------------------------------------------------
//
// SynchronizedBuffer iterators
//
//----------------------------------------------------------------------------

Segments::iterator SynchronizedBuffer::begin() {
    return m_segments.begin();
}

Segments::const_iterator SynchronizedBuffer::cbegin() {
    return m_segments.cbegin();
}

Segments::iterator SynchronizedBuffer::end() {
    return m_segments.end();
}

Segments::const_iterator SynchronizedBuffer::cend() {
    return m_segments.cend();
}

Segments::iterator SynchronizedBuffer::edit_begin() {
    for (auto it = m_segments.begin(); it != m_segments.end(); it++) {
        if (it[0].editing) {
            return it;
        }
    }

    return m_segments.end();
}

Segments::const_iterator SynchronizedBuffer::edit_cbegin() {
    for (auto it = m_segments.cbegin(); it != m_segments.cend(); it++) {
        if (it[0].editing) {
            return it;
        }
    }

    return m_segments.cend();
}

Segments::iterator SynchronizedBuffer::edit_end() {
    for (auto it = m_segments.end() - 1; it != m_segments.begin(); --it) {
        if (it[0].editing) {
            return ++it;
        }
    }

    return m_segments.end();
}

Segments::const_iterator SynchronizedBuffer::edit_cend() {
    for (auto it = m_segments.cend() - 1; it != m_segments.cbegin(); --it) {
        if (it[0].editing) {
            return ++it;
        }
    }

    return m_segments.cend();
}

Segments::iterator SynchronizedBuffer::caret() {
    return m_segments.begin() + m_caret.segment_pos;
}

Segments::const_iterator SynchronizedBuffer::ccaret() {
    return m_segments.cbegin() + m_caret.segment_pos;
}

std::string::iterator SynchronizedBuffer::input_caret() {
    auto s = caret();
    auto it = s[0].input_value.begin() + m_caret.input_pos;
    return it;
}

std::string::const_iterator SynchronizedBuffer::input_ccaret() {
    auto s = ccaret();
    auto it = s[0].input_value.cbegin() + m_caret.input_pos;
    return it;
}

std::string::iterator SynchronizedBuffer::display_caret() {
    auto s = caret();
    auto it = s[0].display_value.begin();
    utf8::advance(it, m_caret.display_pos, s[0].display_value.end());
    return it;
}

std::string::const_iterator SynchronizedBuffer::display_ccaret() {
    auto s = ccaret();
    auto it = s[0].display_value.cbegin();
    utf8::advance(it, m_caret.display_pos, s[0].display_value.cend());
    return it;
}

auto SynchronizedBuffer::empty() -> bool {
    return m_segments.size() == 1 && m_segments[0].input_value.size() == 0;
}

auto SynchronizedBuffer::erase(CursorDirection dir) -> void {
    if (dir == CursorDirection::L) {
        if (m_caret.input_pos == 0) {
            m_caret--;
            return;
        }

        m_caret--;
    }

    erase(1);
}

auto SynchronizedBuffer::insert(char ch) -> void {
    auto it = caret();
    auto &s = *it;
    s.input_value.insert(m_caret.input_pos, 1, ch);
    ++(m_caret.input_pos);
}

auto SynchronizedBuffer::isCursorAtEnd() -> bool {
    return m_caret.IsAtEnd();
};

auto SynchronizedBuffer::MoveCaret(CursorDirection dir) -> void {
    if (dir == CursorDirection::R) {
        m_caret++;
    } else if (dir == CursorDirection::L) {
        m_caret--;
    }
}

auto SynchronizedBuffer::moveCursorToEnd() -> void {
    m_caret.ShiftToEnd();
}

auto SynchronizedBuffer::rawText(Segments::iterator first, Segments::iterator last) -> std::string {
    std::string ret;
    ret.reserve(100);

    while (first != last) {
        auto &s = *(first);
        ret.append(s.input_value);
        ++first;
    }

    return std::move(ret);
}

auto SynchronizedBuffer::segmentCount() -> size_t {
    return m_segments.size();
}

auto SynchronizedBuffer::segmentByCandidate(Segments::iterator first, Segments::iterator last,
                                            const Candidate &candidate) -> void {
    // Save for afterwards
    auto atEnd = m_caret.IsAtEnd();
    auto rawCursorOffset = m_caret.rawOffset(first);
    auto segmentIdx = std::distance(m_segments.begin(), first);

    // Build new segments & re-attach
    auto nextSegments = Segments();
    for (auto &chunk : candidate) {
        nextSegments.push_back(Segment{chunk.raw, displayFromRaw(chunk.raw, chunk.token.input), &chunk.token});
    }
    first = m_segments.erase(first, last);
    m_segments.insert(first, std::make_move_iterator(nextSegments.cbegin()),
                      std::make_move_iterator(nextSegments.cend()));

    nextSegments.clear();

    // Update cursor
    // Case: at the end - stay at the end
    if (atEnd) {
        m_caret.ShiftToEnd();
    } else {
        m_caret.syncToRaw(segmentIdx, rawCursorOffset);
    }

    updateSegmentSpacing();
}

auto SynchronizedBuffer::updateSegmentSpacing() -> void {
    if (m_segments.size() < 2) {
        return;
    }

    auto tokens = std::vector<std::string_view>();
    std::transform(m_segments.cbegin(), m_segments.cend(), std::back_inserter(tokens), [](const Segment &s) {
        return std::string_view(s.display_value);
    });

    auto spaces = tokenSpacer(tokens);

    for (auto &&[spaced, segment] : c9::zip(spaces, m_segments)) {
        segment.spaced = spaced;
    }
}

// Private

auto SynchronizedBuffer::erase(size_t len) -> void {
    auto it = caret();
    auto &s = *it;

    auto r_start = input_caret();
    auto d_start = display_caret();

    auto r_it = std::string::iterator(r_start);
    auto d_it = std::string::iterator(d_start);

    ParallelAdvance(r_it, s.input_value.end(), d_it, s.display_value.end());

    if (hasToneDiacritic(std::string(d_start, d_it))) {
        removeToneFromRawBuffer();
    }

    s.input_value.erase(r_start, r_it);
    if (s.input_value.empty()) {
        eraseSegment(it);
    }

    if (m_segments.size() == 0) {
        m_segments.push_back(Segment());
    }
}

auto SynchronizedBuffer::eraseSegment(Segments::iterator first) -> void {
    auto &s = *first;

    if (caret() == first) {
        m_caret--;
    }

    m_segments.erase(first);
    updateSegmentSpacing();
}

auto SynchronizedBuffer::removeToneFromRawBuffer() -> void {
    auto s_it = caret();
    auto &s = *s_it;

    auto r_it = s.input_value.begin() + m_caret.input_pos;

    static auto isSyllableBreak = [](char ch) {
        return ch == ' ' || ch == '-' || ch == '0';
    };

    // First search to the right
    while (r_it != s.input_value.end() && !isSyllableBreak(*r_it)) {
        if (isdigit(*r_it)) {
            s.input_value.erase(r_it, r_it + 1);
            return;
        }
        ++r_it;
    }

    --r_it;

    // Then search to the left (and update cursor position)
    while (r_it != s.input_value.begin() && !isSyllableBreak(*r_it)) {
        if (isdigit(*r_it)) {
            s.input_value.erase(r_it, r_it + 1);
            m_caret.input_pos--;
        }
    }
}

} // namespace khiin::engine
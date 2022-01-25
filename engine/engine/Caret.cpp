#include <utf8cpp/utf8.h>

#include "Lomaji.h"
#include "SynchronizedBuffer.h"
#include "ParallelIterators.h"

namespace khiin::engine {

void Caret::operator++() {
    if (IsAtEnd()) {
        return;
    }

    auto seg_it = buf->ccaret();
    auto &s = *seg_it;

    if (s.spaced && s.input_value.size() == input_pos) {
        segment_pos++;
        input_pos = 0;
        display_pos = 0;
        return;
    }

    auto r_it = buf->input_ccaret();
    auto d_it = buf->display_ccaret();

    ParallelAdvance(r_it, s.input_value.cend(), d_it, s.display_value.cend());
    input_pos = std::distance(s.input_value.cbegin(), r_it);
    display_pos = utf8::distance(s.display_value.cbegin(), d_it);
}

void Caret::operator++(int) {
    ++(*this);
}

void Caret::operator--() {
    if (segment_pos == 0 && input_pos == 0) {
        return;
    }

    auto s_it = buf->ccaret();

    if (input_pos == 0) {
        s_it--;
        segment_pos--;
        input_pos = s_it[0].input_value.size();
        display_pos = utf8Size(s_it[0].display_value);

        if (!s_it[0].spaced) {
            --input_pos;
            --display_pos;
        }

        return;
    }

    auto &s = *s_it;
    auto r_it = buf->input_ccaret();
    auto d_it = buf->display_ccaret();

    ParallelPrior(r_it, s.input_value.cbegin(), d_it, s.display_value.cbegin());
    input_pos = std::distance(s.input_value.cbegin(), r_it);
    display_pos = utf8::distance(s.display_value.cbegin(), d_it);
}

void Caret::operator--(int) {
    --(*this);
}

bool Caret::IsAtEnd() {
    return segment_pos == buf->m_segments.size() - 1 && input_pos == buf->m_segments.back().input_value.size();
}

void Caret::ShiftToStart() {
    segment_pos = 0;
    input_pos = 0;
    display_pos = 0;
}

void Caret::ShiftToEnd() {
    auto &last = buf->m_segments.back();
    segment_pos = buf->m_segments.size() - 1;
    input_pos = last.input_value.size();
    display_pos = utf8::distance(last.display_value.cbegin(), last.display_value.cend());
}

auto Caret::displayOffset() -> size_t {
    return displayOffset(buf->begin());
}

auto Caret::displayOffset(Segments::iterator from) -> size_t {
    auto len = size_t(0);
    auto targetSegment = buf->caret();

    while (from != targetSegment) {
        auto &s = *from;
        len += utf8::distance(s.display_value.cbegin(), s.display_value.cend());
        if (s.spaced) {
            ++len;
        }
        ++from;
    }

    len += display_pos;
    return len;
}

auto Caret::rawOffset(Segments::iterator from) -> size_t {
    auto len = size_t(0);
    auto targetSegment = buf->caret();

    while (from != targetSegment) {
        len += (*from).input_value.size();
        ++from;
    }

    len += input_pos;
    return len;
}

auto Caret::syncToRaw(size_t segmentStart, size_t rawOffset) -> void {
    auto it = buf->cbegin() + segmentStart;
    while (it != buf->cend() && rawOffset > 0) {
        if ((*it).input_value.size() >= rawOffset) {
            break;
        }

        rawOffset -= (*it).input_value.size();
        ++it;
    }

    auto &seg = *it;
    auto r_it = seg.input_value.begin();
    auto r_target = r_it + rawOffset;
    auto r_end = seg.input_value.end();

    auto d_it = seg.display_value.begin();
    auto d_end = seg.display_value.end();

    while (r_it != r_end && r_it != r_target && d_it != d_end) {
        ParallelAdvance(r_it, r_end, d_it, d_end);
    }

    segment_pos = std::distance(buf->cbegin(), it);
    input_pos = std::distance(seg.input_value.begin(), r_target);
    display_pos = utf8::distance(seg.display_value.begin(), d_it);
}

} // namespace khiin::engine

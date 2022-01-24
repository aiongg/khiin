#include <utf8cpp/utf8.h>

#include "buffer.h"
#include "lomaji.h"

namespace khiin::engine {

void Cursor::operator++() {
    if (atEnd()) {
        return;
    }

    auto seg_it = buf->segmentAtCursor();
    auto &s = *seg_it;

    if (s.spaced && s.raw.size() == raw) {
        segment++;
        raw = 0;
        display = 0;
        return;
    }

    auto r_it = buf->rawCursor();
    auto d_it = buf->displayCursor();

    parallelNext(r_it, s.raw.end(), d_it, s.display.end());
    raw = std::distance(s.raw.begin(), r_it);
    display = utf8::distance(s.display.begin(), d_it);
}

void Cursor::operator++(int) { ++(*this); }

void Cursor::operator--() {
    if (segment == 0 && raw == 0) {
        return;
    }

    auto s_it = buf->segmentAtCursor();

    if (raw == 0) {
        s_it--;
        segment--;
        raw = (*s_it).raw.size();
        display = utf8Size((*s_it).display);

        if (!(*s_it).spaced) {
            --raw;
            --display;
        }

        return;
    }

    auto &s = *s_it;
    auto r_it = buf->rawCursor();
    auto d_it = buf->displayCursor();

    parallelPrior(r_it, s.raw.begin(), d_it, s.display.begin());
    raw = std::distance(s.raw.begin(), r_it);
    display = utf8::distance(s.display.begin(), d_it);
}

void Cursor::operator--(int) { --(*this); }

auto Cursor::atEnd() -> bool {
    return segment == buf->segments.size() - 1 &&
           raw == buf->segments.back().raw.size();
}

auto Cursor::clear() -> void {
    segment = 0;
    raw = 0;
    display = 0;
}

auto Cursor::displayOffset() -> size_t {
    return displayOffset(buf->segmentBegin());
}

auto Cursor::displayOffset(SegmentIter from) -> size_t {
    auto len = size_t(0);
    auto targetSegment = buf->segmentAtCursor();

    while (from != targetSegment) {
        auto &s = *from;
        len += utf8::distance(s.display.cbegin(), s.display.cend());
        if (s.spaced) {
            ++len;
        }
        ++from;
    }

    len += display;
    return len;
}

auto Cursor::rawOffset(SegmentIter from) -> size_t {
    auto len = size_t(0);
    auto targetSegment = buf->segmentAtCursor();

    while (from != targetSegment) {
        len += (*from).raw.size();
        ++from;
    }

    len += raw;
    return len;
}

auto Cursor::setToEnd() -> void {
    auto &last = buf->segments.back();
    segment = buf->segments.size() - 1;
    raw = last.raw.size();
    display = utf8::distance(last.display.cbegin(), last.display.cend());
}

auto Cursor::syncToRaw(size_t segmentStart, size_t rawOffset) -> void {
    auto it = buf->segments.begin() + segmentStart;
    while (it != buf->segments.end() && rawOffset > 0) {
        if ((*it).raw.size() >= rawOffset) {
            break;
        }

        rawOffset -= (*it).raw.size();
        ++it;
    }

    auto &seg = *it;
    auto r_it = seg.raw.begin();
    auto r_target = r_it + rawOffset;
    auto r_end = seg.raw.end();

    auto d_it = seg.display.begin();
    auto d_end = seg.display.end();

    while (r_it != r_end && r_it != r_target && d_it != d_end) {
        parallelNext(r_it, r_end, d_it, d_end);
    }

    segment = std::distance(buf->segments.begin(), it);
    raw = std::distance(seg.raw.begin(), r_target);
    display = utf8::distance(seg.display.begin(), d_it);
}

} // namespace khiin::engine

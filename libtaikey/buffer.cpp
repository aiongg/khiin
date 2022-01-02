#include <algorithm>

#include <utf8cpp/utf8.h>

#include "buffer.h"

namespace TaiKey {

Buffer::Buffer()
    : text(""), cursor(0), sylOffsets(std::vector<size_t>()),
      candOffsets(std::vector<size_t>()), editingRange(std::make_pair(0, 0)) {}

auto Buffer::cursorIt() -> std::string::iterator {
    auto it = text.begin();
    utf8::advance(it, cursor, text.end());
    return it;
}

auto Buffer::ccursorIt() -> std::string::const_iterator {
    auto it = text.cbegin();
    utf8::advance(it, cursor, text.cend());
    return it;
}

auto Buffer::candBegin() -> std::string::iterator {
    if (candOffsets.size() < 2) {
        return text.begin();
    }

    auto it =
        std::upper_bound(candOffsets.cbegin(), candOffsets.cend(), cursor) - 1;

    auto ret = text.begin();
    utf8::advance(ret, *it, text.end());
    return ret;
}

auto Buffer::sylBegin() -> std::string::iterator {
    if (sylOffsets.size() < 2) {
        return text.begin();
    }

    auto it =
        std::upper_bound(sylOffsets.cbegin(), sylOffsets.cend(), cursor) - 1;

    auto ret = text.begin();
    utf8::advance(ret, *it, text.end());
    return ret;
}

auto Buffer::sylEnd() -> std::string::iterator {
    if (sylOffsets.size() < 2) {
        return text.end();
    }

    auto it = std::upper_bound(sylOffsets.cbegin(), sylOffsets.cend(), cursor);

    if (it == sylOffsets.cend()) {
        return text.end();
    }

    auto ret = text.begin();
    utf8::advance(ret, *it, text.end());
    return ret;
}

} // namespace TaiKey
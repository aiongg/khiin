#pragma once

#include <string>
#include <vector>

namespace TaiKey {

class Buffer {
  public:
    Buffer();
    std::string text;
    size_t cursor;
    std::vector<size_t> sylOffsets;
    std::vector<size_t> candOffsets;
    std::pair<size_t, size_t> editingRange;

    auto candBegin() -> std::string::iterator;
    auto cursorIt() -> std::string::iterator;
    auto ccursorIt() -> std::string::const_iterator;
    auto sylBegin() -> std::string::iterator;
    auto sylEnd() -> std::string::iterator;
};

class SynchronizedBuffer {
    Buffer raw;
    Buffer display;
};

} // namespace TaiKey
#pragma once

#include <utf8cpp/utf8.h>

namespace khiin::engine {

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

template <typename T>
static inline utf8_size_t Utf8Size(T string) {
    return static_cast<utf8_size_t>(utf8::distance(string.cbegin(), string.cend()));
}

std::string toNFD(std::string_view s);
std::string toNFC(std::string_view s);

} // namespace khiin::engine
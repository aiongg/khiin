#include "unicode_utils.h"

#include <unilib/uninorms.h>
#include <utf8cpp/utf8/cpp17.h>

namespace khiin::engine {

std::string toNFD(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfd(u32s);
    return std::move(utf8::utf32to8(u32s));
}

std::string toNFC(std::string_view s) {
    auto u32s = utf8::utf8to32(s);
    ufal::unilib::uninorms::nfc(u32s);
    return std::move(utf8::utf32to8(u32s));
}

} // namespace khiin::engine

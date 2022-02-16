#include "Lomaji.h"

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

#include "unicode_utils.h"

namespace khiin::engine {
namespace {

bool IsNoncursorableCodepoint(uint32_t cp) {
    return 0x0300 <= cp && cp <= 0x0358;
}

} // namespace

utf8_size_t Lomaji::MoveCaret(std::string_view str, utf8_size_t start_pos, CursorDirection dir) {
    auto str_size = unicode::u8_size(str);
    if (start_pos > str_size) {
        return str_size;
    }

    auto it = str.cbegin();
    if (dir == CursorDirection::L) {
        if (start_pos == 0) {
            return 0;
        }

        utf8::unchecked::advance(it, start_pos);

        while (it != str.cbegin()) {
            auto cp = utf8::unchecked::prior(it);
            if (IsNoncursorableCodepoint(cp)) {
                continue;
            } else {
                break;
            }
        }
    } else if (dir == CursorDirection::R) {
        if (start_pos == unicode::u8_size(str)) {
            return start_pos;
        }

        utf8::unchecked::advance(it, start_pos + 1);

        while (it != str.cend()) {
            auto cp = utf8::unchecked::peek_next(it);
            if (IsNoncursorableCodepoint(cp)) {
                utf8::unchecked::next(it);
                continue;
            } else {
                break;
            }
        }
    }

    return utf8::unchecked::distance(str.cbegin(), it);
}

bool Lomaji::IsLomaji(std::string_view str) {
    auto it = str.begin();
    while (it != str.end()) {
        auto cp = utf8::unchecked::next(it);
        if (cp >= unicode::kHanjiCutoff) {
            return false;
        }
    }
    return true;
}

} // namespace khiin::engine

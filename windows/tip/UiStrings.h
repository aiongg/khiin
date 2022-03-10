#pragma once

#include <string>

namespace khiin::win32 {
enum class UiLanguage;
}

namespace khiin::win32::strings {

std::string T(uint32_t str_rid, UiLanguage lang);

} // namespace khiin::win32::strings

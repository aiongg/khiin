#pragma once

#include <string>

namespace khiin::proto {
enum UiLanguage : int;
}

namespace khiin::win32::strings {

std::string T(uint32_t str_rid, proto::UiLanguage lang);

} // namespace khiin::win32::strings

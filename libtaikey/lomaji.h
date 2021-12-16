#pragma once

#include <boost/regex.hpp>

#include "common.h"
#include "errors.h"

namespace TaiKey {

using str_iter = std::string::iterator;
using str_range = std::pair<str_iter, str_iter>;

std::string toNFC(std::string s);
std::string toNFD(std::string s);

Tone getToneFromDigit(char ch);
Tone getToneFromTelex(char ch);

std::string asciiToUtf8(std::string ascii, Tone tone, bool khin);

std::string placeToneOnSyllable(std::string u8syllable, Tone tone);

std::string stripDiacritics(std::string s);

Tone checkTone78Swap(std::string u8syllable, Tone tone);

str_range getAsciiSelectionFromUtf8(std::string ascii, std::string u8str,
                                    str_iter u8begin, str_iter u8end);

size_t getAsciiCursorFromUtf8(std::string ascii, std::string u8str,
                                size_t u8cursor);

} // namespace TaiKey
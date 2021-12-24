#pragma once

#include <boost/regex.hpp>

#include "common.h"
#include "errors.h"

namespace TaiKey {

using str_iter = std::string::iterator;
using str_range = std::pair<str_iter, str_iter>;

auto toNFC(std::string s) -> std::string;
auto toNFD(std::string s) -> std::string;
auto utf8Size(std::string s) -> size_t;

auto getToneFromDigit(char ch) -> Tone;
auto getToneFromTelex(char ch) -> Tone;

auto asciiToUtf8(std::string ascii, Tone tone, bool khin) -> std::string;

auto utf8ToAsciiLower(std::string u8string) -> std::string;

auto placeToneOnSyllable(std::string u8syllable, Tone tone) -> std::string;

auto stripDiacritics(std::string s);

auto checkTone78Swap(std::string u8syllable, Tone tone) -> Tone;

auto getAsciiCursorFromUtf8(std::string ascii, std::string u8str,
                            size_t u8cursor) -> size_t;

auto spaceAsciiByLomaji(std::string ascii, std::string lomaji) -> VStr;

} // namespace TaiKey
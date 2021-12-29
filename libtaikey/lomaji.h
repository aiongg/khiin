#pragma once

#include "common.h"
#include "errors.h"

namespace TaiKey {

auto asciiSyllableToUtf8(std::string ascii) -> std::string;

auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin)
    -> std::string;

auto checkTone78Swap(std::string u8syllable, Tone tone) -> Tone;

auto getAsciiCursorFromUtf8(std::string ascii, std::string u8str,
                            size_t u8cursor) -> size_t;

auto getToneFromDigit(char ch) -> Tone;

auto getToneFromTelex(char ch) -> Tone;

auto parallelNext(std::string_view ascii, size_t i, std::string_view u8string,
                   Utf8Size j) -> std::pair<size_t, Utf8Size>;

auto parallelPrior(std::string_view ascii, size_t i, std::string_view u8string,
                   Utf8Size j) -> std::pair<size_t, Utf8Size>;

auto placeToneOnSyllable(std::string u8syllable, Tone tone) -> std::string;

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji)
    -> VStr;

auto stripDiacritics(std::string s);

auto toNFC(std::string s) -> std::string;

auto toNFD(std::string s) -> std::string;

auto utf8Size(std::string s) -> Utf8Size;

auto utf8ToAsciiLower(std::string u8string) -> std::string;

} // namespace TaiKey
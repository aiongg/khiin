#pragma once

#include "common.h"
#include "errors.h"

namespace TaiKey {

auto asciiSyllableToUtf8(std::string ascii) -> std::string;

auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin)
    -> std::string;

auto checkTone78Swap(std::string u8syllable, Tone tone) -> Tone;

auto getToneFromDigit(char ch) -> Tone;

auto getToneFromTelex(char ch) -> Tone;

auto hasToneDiacritic(std::string sv) -> bool;

auto parallelNext(std::string::iterator &a_it, std::string::iterator &a_end,
                  std::string::iterator &u_it, std::string::iterator &u_end)
    -> void;

auto parallelPrior(std::string::iterator &a_it, std::string::iterator &a_end,
                   std::string::iterator &u_it, std::string::iterator &u_end)
    -> void;

auto stripDiacritics(std::string s);

auto placeToneOnSyllable(std::string u8syllable, Tone tone) -> std::string;

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji) -> VStr;

auto toNFC(std::string_view s) -> std::string;

auto toNFD(std::string_view s) -> std::string;

auto utf8Size(std::string s) -> Utf8Size;

auto utf8ToAsciiLower(std::string u8string) -> std::string;

} // namespace TaiKey
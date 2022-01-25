#pragma once

#include "common.h"
#include "errors.h"

namespace khiin::engine {

auto asciiSyllableToUtf8(std::string ascii) -> std::string;

auto asciiSyllableToUtf8(std::string ascii, Tone tone, bool khin) -> std::string;

auto checkTone78Swap(std::string u8syllable, Tone tone) -> Tone;

auto getToneFromDigit(char ch) -> Tone;

auto getToneFromTelex(char ch) -> Tone;

auto hasToneDiacritic(std::string sv) -> bool;

auto stripDiacritics(std::string s);

auto placeToneOnSyllable(std::string u8syllable, Tone tone) -> std::string;

auto spaceAsciiByUtf8(std::string ascii, std::string lomaji) -> string_vector;

auto tokenSpacer(std::vector<std::string_view> tokens) -> std::vector<bool>;

auto toNFC(std::string_view s) -> std::string;

auto toNFD(std::string_view s) -> std::string;

auto utf8back(std::string_view s) -> uint32_t;

auto utf8first(std::string_view s) -> uint32_t;

auto utf8Size(std::string s) -> utf8_size_t;

auto utf8ToAsciiLower(std::string u8string) -> std::string;

} // namespace khiin::engine

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace TaiKey {

enum class Tone {
    NaT,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    T8,
    T9,
    TK,
};

const std::unordered_map<Tone, char> TONE_TELEX_MAP = {
    {Tone::T2, 's'}, {Tone::T3, 'f'}, {Tone::T5, 'l'}, {Tone::T7, 'j'},
    {Tone::T8, 'j'}, {Tone::T9, 'w'}, {Tone::TK, 'q'}};

const std::unordered_map<Tone, char> TONE_DIGIT_MAP = {
    {Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'}, {Tone::T7, '7'},
    {Tone::T8, '8'}, {Tone::T9, '9'}, {Tone::TK, '0'}};

const std::unordered_map<Tone, std::string> TONE_UTF_MAP = {
    {Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"}, {Tone::T5, u8"\u0302"},
    {Tone::T6, u8"\u030c"}, {Tone::T7, u8"\u0304"}, {Tone::T8, u8"\u030d"},
    {Tone::T9, u8"\u0306"}, {Tone::TK, u8"\u00b7"}};

const std::string TONES[] = {u8"\u0301", u8"\u0300", u8"\u0302", u8"\u030c",
                             u8"\u0304", u8"\u030d", u8"\u0306", u8"\u00b7"};

const std::unordered_set<char> PTKH = {'p', 't', 'k', 'h'};

} // namespace TaiKey

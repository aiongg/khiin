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

const std::string U8_NN = u8"\u207f";
const std::string U8_OU = u8"\u0358";
const std::string U8_T2 = u8"\u0301";
const std::string U8_T3 = u8"\u0300";
const std::string U8_T5 = u8"\u0302";
const std::string U8_T7 = u8"\u0304";
const std::string U8_T8 = u8"\u030D";
const std::string U8_T9 = u8"\u0306";
const std::string U8_TK = u8"\u00b7";

const std::unordered_map<Tone, char> TONE_TELEX_MAP = {
    {Tone::T2, 's'}, {Tone::T3, 'f'}, {Tone::T5, 'l'}, {Tone::T7, 'j'},
    {Tone::T8, 'j'}, {Tone::T9, 'w'}, {Tone::TK, 'q'}};

const std::unordered_map<Tone, char> TONE_DIGIT_MAP = {
    {Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'}, {Tone::T7, '7'},
    {Tone::T8, '8'}, {Tone::T9, '9'}, {Tone::TK, '0'}};

const std::unordered_map<Tone, std::string> TONE_UTF_MAP = {
    {Tone::T2, U8_T2}, {Tone::T3, U8_T3}, {Tone::T5, U8_T5},
    {Tone::T7, U8_T7}, {Tone::T8, U8_T8}, {Tone::T9, U8_T9},
    {Tone::TK, U8_TK}};

const std::string TONES[] = {u8"\u0301", u8"\u0300", u8"\u0302", u8"\u030c",
                             u8"\u0304", u8"\u030d", u8"\u0306", u8"\u00b7"};

const std::unordered_set<char> PTKH = {'P', 'T', 'K', 'H', 'p', 't', 'k', 'h'};

} // namespace TaiKey

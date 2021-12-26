#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace TaiKey {

using VStr = std::vector<std::string>;

using Utf8Size = size_t; // number of UTF codepoints, 1-4 bytes

//using AlignedCursor = std::pair<size_t, Utf8Size>;

struct AlignedText {
    size_t aBegin = 0;
    size_t aEnd = 0;
    Utf8Size uBegin = 0;
    Utf8Size uEnd = 0;
};

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
const std::string U8_R = u8"\u0324";

const uint32_t U32_NN = 0x207f;
const uint32_t U32_OU = 0x0358;
const uint32_t U32_T2 = 0x0301;
const uint32_t U32_T3 = 0x0300;
const uint32_t U32_T5 = 0x0302;
const uint32_t U32_T7 = 0x0304;
const uint32_t U32_T8 = 0x030D;
const uint32_t U32_T9 = 0x0306;
const uint32_t U32_TK = 0x00b7;
const uint32_t U32_R = 0x0324;

const std::unordered_map<Tone, char> ToneToTelexMap = {
    {Tone::T2, 's'}, {Tone::T3, 'f'}, {Tone::T5, 'l'}, {Tone::T7, 'j'},
    {Tone::T8, 'j'}, {Tone::T9, 'w'}, {Tone::TK, 'q'}};

const std::unordered_map<Tone, char> ToneToDigitMap = {
    {Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'}, {Tone::T7, '7'},
    {Tone::T8, '8'}, {Tone::T9, '9'}, {Tone::TK, '0'}};

const std::unordered_map<Tone, std::string> ToneToUtf8Map = {
    {Tone::T2, U8_T2}, {Tone::T3, U8_T3}, {Tone::T5, U8_T5},
    {Tone::T7, U8_T7}, {Tone::T8, U8_T8}, {Tone::T9, U8_T9},
    {Tone::TK, U8_TK}};

static std::unordered_map<uint32_t, std::string> ToneUint32ToDigitMap = {
    {U32_T2, "2"}, {U32_T3, "3"}, {U32_T5, "5"}, {U32_T7, "7"}, {U32_T8, "8"},
    {U32_T9, "9"}, {U32_TK, "0"}, {U32_R, "r"},  {U32_OU, "u"}, {U32_NN, "nn"}};

const std::unordered_map<Tone, uint32_t> ToneToUint32Map = {
    {Tone::T2, U32_T2}, {Tone::T3, U32_T3}, {Tone::T5, U32_T5}, {Tone::T7, U32_T7},
    {Tone::T8, U32_T8}, {Tone::T9, U32_T9}, {Tone::TK, U32_TK}};

const std::string TONES[] = {U8_T2, U8_T3, U8_T5, u8"\u030c",
                             U8_T7, U8_T8, u8"\u0306", U8_TK};

const std::unordered_set<char> PTKH = {'P', 'T', 'K', 'H', 'p', 't', 'k', 'h'};

} // namespace TaiKey

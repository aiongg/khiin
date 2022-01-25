#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace khiin::engine {

inline const std::string kKhiinHome = "KHIIN_HOME";

using string_vector = std::vector<std::string>;

using utf8_size_t = size_t; // number of UTF codepoints, 1-4 bytes

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

inline const std::string U8_NN = u8"\u207f";
inline const std::string U8_OU = u8"\u0358";
inline const std::string U8_T2 = u8"\u0301";
inline const std::string U8_T3 = u8"\u0300";
inline const std::string U8_T5 = u8"\u0302";
inline const std::string U8_T7 = u8"\u0304";
inline const std::string U8_T8 = u8"\u030D";
inline const std::string U8_T9 = u8"\u0306";
inline const std::string U8_TK = u8"\u00b7";
inline const std::string U8_R = u8"\u0324";
inline const std::string U8_UR = u8"\u1e73";

inline const uint32_t U32_NN = 0x207f;
inline const uint32_t U32_OU = 0x0358;
inline const uint32_t U32_T2 = 0x0301;
inline const uint32_t U32_T3 = 0x0300;
inline const uint32_t U32_T5 = 0x0302;
inline const uint32_t U32_T7 = 0x0304;
inline const uint32_t U32_T8 = 0x030D;
inline const uint32_t U32_T9 = 0x0306;
inline const uint32_t U32_TK = 0x00B7;
inline const uint32_t U32_R = 0x0324;
inline const uint32_t U32_NN_UC = 0x1D3A;
inline const uint32_t U32_UR_UC = 0x1E72;
inline const uint32_t U32_UR = 0x1E73;

inline const std::unordered_set<uint32_t> TwoAsciiCodepoints = {
    U32_TK, U32_NN, U32_NN_UC, U32_UR, U32_UR_UC};

inline const std::unordered_map<Tone, char> ToneToTelexMap = {
    {Tone::T2, 's'}, {Tone::T3, 'f'}, {Tone::T5, 'l'}, {Tone::T7, 'j'},
    {Tone::T8, 'j'}, {Tone::T9, 'w'}, {Tone::TK, 'q'}};

inline const std::unordered_map<Tone, char> ToneToDigitMap = {
    {Tone::T2, '2'}, {Tone::T3, '3'}, {Tone::T5, '5'}, {Tone::T7, '7'},
    {Tone::T8, '8'}, {Tone::T9, '9'}, {Tone::TK, '0'}};

inline const std::unordered_map<Tone, std::string> ToneToUtf8Map = {
    {Tone::T2, U8_T2}, {Tone::T3, U8_T3}, {Tone::T5, U8_T5}, {Tone::T7, U8_T7},
    {Tone::T8, U8_T8}, {Tone::T9, U8_T9}, {Tone::TK, U8_TK}};

inline static std::unordered_map<uint32_t, std::string> ToneUint32ToDigitMap = {
    {U32_T2, "2"}, {U32_T3, "3"}, {U32_T5, "5"}, {U32_T7, "7"}, {U32_T8, "8"},
    {U32_T9, "9"}, {U32_TK, "0"}, {U32_R, "r"},  {U32_OU, "u"}, {U32_NN, "nn"}};

inline const std::unordered_map<Tone, uint32_t> ToneToUint32Map = {
    {Tone::T2, U32_T2}, {Tone::T3, U32_T3}, {Tone::T5, U32_T5},
    {Tone::T7, U32_T7}, {Tone::T8, U32_T8}, {Tone::T9, U32_T9},
    {Tone::TK, U32_TK}};

inline const std::string TONES[] = {U8_T2, U8_T3, U8_T5, u8"\u030c",
                             U8_T7, U8_T8, u8"\u0306", U8_TK};

inline const auto U32_TONES =
    std::vector<uint32_t>{U32_T2, U32_T3, U32_T5, U32_T7, U32_T8, U32_T9};

inline const std::unordered_set<char> PTKH = {'P', 'T', 'K', 'H', 'p', 't', 'k', 'h'};

} // namespace khiin::engine

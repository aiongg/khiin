#include "Lomaji.h"

#include <array>
#include <functional>

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp17.h>

#include "utils/unicode.h"

namespace khiin::engine {
namespace {
namespace u8u = utf8::unchecked;
using namespace unicode;

inline constexpr std::array<char *, 7> kOrderedToneablesIndex1 = {{"o", "a", "e", "u", "i", "ng", "m"}};
inline constexpr std::array<char *, 2> kOrderedToneablesIndex2 = {{"oa", "oe"}};
inline constexpr std::array<char, 7> kToneableLetters = {'a', 'e', 'i', 'm', 'n', 'o', 'u'};
const std::string kKhinDotStr = u8"\u00b7";
const std::string kKhinHyphenStr = "--";

const std::unordered_map<Tone, std::string> kToneToUnicodeMap = {{Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"},
                                                                 {Tone::T5, u8"\u0302"}, {Tone::T7, u8"\u0304"},
                                                                 {Tone::T8, u8"\u030d"}, {Tone::T9, u8"\u0306"}};

bool IsCombiningCharacter(uint32_t cp) {
    return 0x0300 <= cp && cp <= 0x0358;
}

inline constexpr char32_t kNasal = 0x207f;
inline constexpr char32_t kNasalUpper = 0x1d3a;

inline bool IsUpper(char32_t c) {
    return c <= kMaxAscii && isupper(c);
}

inline bool IsAlpha(char32_t c) {
    return c <= kMaxAscii && isalpha(c);
}

inline bool BothAlpha(char32_t c1, char32_t c2) {
    return c1 <= kMaxAscii && c2 <= kMaxAscii && isalpha(c1) && isalpha(c2);
}

constexpr std::initializer_list<std::pair<const unsigned int, const char *>> kLatinDecompositionData = {
    {U'À', u8"A\u0300"}, {U'Á', u8"A\u0301"}, {U'Â', u8"A\u0302"}, {U'È', u8"E\u0300"}, {U'É', u8"E\u0301"},
    {U'Ê', u8"E\u0302"}, {U'Ì', u8"I\u0300"}, {U'Í', u8"I\u0301"}, {U'Î', u8"I\u0302"}, {U'Ò', u8"O\u0300"},
    {U'Ó', u8"O\u0301"}, {U'Ô', u8"O\u0302"}, {U'Ù', u8"U\u0300"}, {U'Ú', u8"U\u0301"}, {U'Û', u8"U\u0302"},
    {U'à', u8"a\u0300"}, {U'á', u8"a\u0301"}, {U'â', u8"a\u0302"}, {U'è', u8"e\u0300"}, {U'é', u8"e\u0301"},
    {U'ê', u8"e\u0302"}, {U'ì', u8"i\u0300"}, {U'í', u8"i\u0301"}, {U'î', u8"i\u0302"}, {U'ò', u8"o\u0300"},
    {U'ó', u8"o\u0301"}, {U'ô', u8"o\u0302"}, {U'ù', u8"u\u0300"}, {U'ú', u8"u\u0301"}, {U'û', u8"u\u0302"},
    {U'Ā', u8"A\u0304"}, {U'ā', u8"a\u0304"}, {U'Ă', u8"A\u0306"}, {U'ă', u8"a\u0306"}, {U'Ē', u8"E\u0304"},
    {U'ē', u8"e\u0304"}, {U'Ĕ', u8"E\u0306"}, {U'ĕ', u8"e\u0306"}, {U'Ī', u8"I\u0304"}, {U'ī', u8"i\u0304"},
    {U'Ĭ', u8"I\u0306"}, {U'ĭ', u8"i\u0306"}, {U'Ń', u8"N\u0301"}, {U'ń', u8"n\u0301"}, {U'Ō', u8"O\u0304"},
    {U'ō', u8"o\u0304"}, {U'Ŏ', u8"O\u0306"}, {U'ŏ', u8"o\u0306"}, {U'Ū', u8"U\u0304"}, {U'ū', u8"u\u0304"},
    {U'Ŭ', u8"U\u0306"}, {U'ŭ', u8"u\u0306"}, {U'Ǹ', u8"N\u0300"}, {U'ǹ', u8"n\u0300"}, {U'Ḿ', u8"M\u0301"},
    {U'ḿ', u8"m\u0301"}, {U'Ṳ', u8"U\u0324"}, {U'ṳ', u8"u\u0324"}};
static const std::unordered_map<unsigned int, const char *> kDecompositionMap{kLatinDecompositionData};

// 0x00e1 > 0x61, 0x0301

} // namespace

utf8_size_t Lomaji::MoveCaret(std::string_view str, utf8_size_t start_pos, CursorDirection dir) {
    auto str_size = u8_size(str);
    if (start_pos > str_size) {
        return str_size;
    }

    auto it = str.cbegin();
    if (dir == CursorDirection::L) {
        if (start_pos == 0) {
            return 0;
        }

        u8u::advance(it, start_pos);

        while (it != str.cbegin()) {
            auto cp = u8u::prior(it);
            if (IsCombiningCharacter(cp)) {
                continue;
            } else {
                break;
            }
        }
    } else if (dir == CursorDirection::R) {
        if (start_pos == u8_size(str)) {
            return start_pos;
        }

        u8u::advance(it, start_pos + 1);

        while (it != str.cend()) {
            auto cp = u8u::peek_next(it);
            if (IsCombiningCharacter(cp)) {
                u8u::advance(it, 1);
                continue;
            } else {
                break;
            }
        }
    }

    return u8u::distance(str.cbegin(), it);
}

bool Lomaji::IsLomaji(std::string_view str) {
    auto it = str.begin();
    while (it != str.end()) {
        auto cp = u8u::next(it);
        if (cp >= kHanjiCutoff) {
            return false;
        }
    }
    return true;
}

std::string Lomaji::Decompose(std::string_view str) {
    auto ret = std::string();
    auto it = u8_begin(str);
    auto end = u8_end(str);
    for (; it != end; ++it) {
        if (auto &found = kDecompositionMap.find(*it); found != kDecompositionMap.end()) {
            ret.append(found->second);
        } else {
            utf8::append(static_cast<char32_t>(*it), ret);
        }
    }
    return ret;
}

// Synchronizes the capitalization from |pattern| with the string |output|
std::string Lomaji::MatchCapitalization(std::string_view pattern, std::string_view output) {
    if (pattern.empty() || output.empty()) {
        return std::string();
    }

    auto ret = std::u32string();

    auto u32_pattern = u8_to_u32_nfd(pattern);
    auto p_it = u32_pattern.begin();
    auto p_end = u32_pattern.end();

    auto u32_output = u8_to_u32_nfd(output);
    auto o_it = u32_output.begin();
    auto o_end = u32_output.end();

    while (p_it != p_end && o_it != o_end) {
        while (p_it != p_end && !IsAlpha(*p_it)) {
            ++p_it;
        }

        while (o_it != o_end && *o_it != kNasal && !IsAlpha(*o_it)) {
            ret.push_back(*o_it);
            ++o_it;
        }

        if (p_it == p_end || o_it == o_end) {
            break;
        }

        if (BothAlpha(*p_it, *o_it)) {
            if (tolower(*p_it) == tolower(*o_it)) {
                ret.push_back(*p_it);
                ++p_it;
                ++o_it;
                continue;
            } else {
                break;
            }
        }

        if (*o_it == kNasal && IsUpper(*p_it)) {
            ret.push_back(kNasalUpper);
            ++o_it;

            auto tmp = tolower(*p_it);
            ++p_it;
            if (p_it != p_end && tolower(*p_it) == tmp) {
                ++p_it;
            }

            continue;
        }

        ret.push_back(*o_it);
        u8u::advance(p_it, 1);
        u8u::advance(o_it, 1);
    }

    while (o_it != o_end) {
        ret.push_back(*o_it);
        ++o_it;
    }

    return u32_to_u8_nfc(ret);
}

bool Lomaji::HasToneable(std::string_view str) {
    for (auto &c1 : str) {
        for (auto &c2 : kToneableLetters) {
            if (tolower(c1) == c2) {
                return true;
            }
        }
    }

    return false;
}

bool Lomaji::HasToneDiacritic(std::string_view str) {
    auto s = Decompose(str);
    auto it = s.cbegin();
    while (it != s.cend()) {
        auto cp = u8u::next(it);
        if (is_tone(cp)) {
            return true;
        }
    }
    return false;
}

size_t Lomaji::FindTonePosition(std::string_view syllable) {
    auto str = unicode::copy_str_tolower(syllable);

    for (auto &sequence : kOrderedToneablesIndex2) {
        if (auto i = str.find(sequence); i != std::string::npos && str.size() > i + 2) {
            return i + 2;
        }
    }

    for (auto &letter : kOrderedToneablesIndex1) {
        if (auto i = str.find(letter); i != std::string::npos) {
            return i + 1;
        }
    }

    return std::string::npos;
}

void Lomaji::ApplyToneDiacritic(Tone tone, std::string &syllable) {
    if (tone == Tone::NaT || tone == Tone::T1 || tone == Tone::T4) {
        return;
    }

    size_t tone_index = FindTonePosition(syllable);
    if (tone_index == std::string::npos) {
        return;
    }

    syllable.insert(tone_index, kToneToUnicodeMap.at(tone));
}

Tone Lomaji::RemoveToneDiacritic(std::string &syllable) {
    if (syllable.empty()) {
        return Tone::NaT;
    }

    auto nfd = Decompose(syllable);
    for (auto &[t, t_str] : kToneToUnicodeMap) {
        if (auto i = nfd.find(t_str); i != std::string::npos) {
            nfd.erase(i, t_str.size());
            syllable = nfd;
            return t;
        }
    }
    return Tone::NaT;
}

bool Lomaji::RemoveKhin(std::string &syllable) {
    if (syllable.empty()) {
        return false;
    }

    auto it = syllable.begin();
    auto cp = u8u::next(it);
    auto has_khin = false;

    if (is_khin(cp)) {
        has_khin = true;
    } else if (u8_size(syllable) > 1 && cp == '-' && u8u::next(it) == '-') {
        has_khin = true;
    }

    if (has_khin) {
        syllable.erase(syllable.begin(), it);
    }

    return has_khin;
}

void Lomaji::ReplaceKhinDot(std::string &str) {
    if (auto pos = str.find(kKhinDotStr); pos != std::string::npos) {
        str.replace(pos, kKhinDotStr.size(), kKhinHyphenStr);
    }
}

} // namespace khiin::engine

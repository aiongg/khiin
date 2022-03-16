#include "pch.h"

#include "Strings.h"

#include "proto/proto.h"
#include "tip/Config.h"

#include "Application.h"
#include "resource.h"

namespace khiin::win32::settings {
namespace {
using namespace proto;

using StrMap = std::unordered_map<uint32_t, std::wstring>;

// clang-format off
StrMap kStringsEn = {
    // Appearance tab
    {IDS_WINDOW_CAPTION,             L"Khíín Taiwanese IME Settings"},
    {IDD_APPEARANCETAB,              L"Display"},
    {IDL_COLOR,                      L"Colors:"},
    {IDS_LIGHT_THEME,                L"Light"},
    {IDS_DARK_THEME,                 L"Dark"},
    {IDL_CANDIDATE_SIZE,             L"Candidate font size:"},
    {IDL_CANDIDATE_SIZE_S,           L"Smaller"},
    {IDL_CANDIDATE_SIZE_L,           L"Larger"},
    {IDL_DISPLAY_LANGUAGE,           L"Display language (介面語言):"},
    {IDS_DISPLAY_LANGUAGE_EN,        L"English"},
    {IDS_DISPLAY_LANGUAGE_HANLO,     L"漢羅台 (Hanlo Taiwanese)"},
    {IDS_DISPLAY_LANGUAGE_LO,        L"Lô-jī Tâi (Romanized Taiwanese)"},
    {IDL_EDIT_TRY,                   L"Try it:"},
    
    // Input tab
    {IDD_INPUTTAB,                   L"Input"},
    {IDL_INPUTMODE,                  L"Input Mode"},
    {IDC_INPUTMODE_CONTINUOUS,       L"Continuous: just keep typing"},
    {IDC_INPUTMODE_BASIC,            L"Basic: one word at a time"},
    {IDC_INPUTMODE_PRO,              L"Manual: no assistance from the IME"},

    {IDL_INPUTMODE_HOTKEY,            L"Switch mode:"},
    {IDS_INPUTMODE_KEY_CTRL_PERIOD,   L"ctrl + ."},
    {IDS_INPUTMODE_KEY_CTRL_BACKTICK, L"ctrl + ` (~)"},

    {IDL_ON_OFF_HOTKEY,              L"IME quick on/off:"},
    {IDS_ON_OFF_HOTKEY_ALTBACKTICK,  L"alt + ` (~)"},
    {IDS_ON_OFF_HOTKEY_SHIFT,        L"shift"},

    {IDL_TONE_KEYS,                  L"Tone keys:"},
    {IDS_TONE_KEYS_NUMERIC,          L"Numeric: 2 3 5 7 8 9 0"},
    {IDS_TONE_KEYS_TELEX,            L"Telex: s f l j j w q"},

    {IDL_DOTTED_O_KEY,               L"Input o͘ :"},
    {IDS_DOTTED_O_OU,                L"ou"},
    {IDS_DOTTED_O_OO,                L"oo"},
    {IDS_DOTTED_O_Y,                 L"y"},

    {IDL_NASAL_KEY,                  L"Input ⁿ :"},
    {IDS_NASAL_NN,                   L"nn"},
    {IDS_NASAL_V,                    L"v"},
    {IDC_OPTION_UPPERCASE_NASAL,     L"Use upper ᴺ"},

    {IDC_OPTION_DOTTED_KHIN,         L"Convert -- to · (khin dot)"},
    {IDC_OPTION_AUTOKHIN,            L"Auto khin following syllables"},
    {IDC_OPTION_EASY_CH,             L"EZ ch (type c for ch)"},

    {IDL_DEFAULT_PUNCTUATION,        L"Default Punctuation"},
    {IDS_PUNCT_FULL_WIDTH,           L"Full width (。、！)"},
    {IDS_PUNCT_HALF_WIDTH,           L"Half width (. , !)"},

    {IDL_HYPHEN_KEY,                 L"Input -:"},
    {IDS_HYPHEN_KEY_HYPHEN,          L"-"},
    {IDS_HYPHEN_KEY_V,               L"v"},
};

StrMap kStringsHanlo = {
    // Appearance tab
    {IDS_WINDOW_CAPTION,             L"起引台語打字法設置"},
    {IDD_APPEARANCETAB,              L"外皮"},
    {IDL_COLOR,                      L"色水："},
    {IDS_LIGHT_THEME,                L"白底"},
    {IDS_DARK_THEME,                 L"烏底"},
    {IDL_CANDIDATE_SIZE,             L"揀字大細："},
    {IDL_CANDIDATE_SIZE_S,           L"Khah 細"},
    {IDL_CANDIDATE_SIZE_L,           L"Khah 大"},
    {IDL_EDIT_TRY,                   L"打看覓仔："},
    {IDL_DISPLAY_LANGUAGE,           L"介面語言 (Display Language)："},
    {IDS_DISPLAY_LANGUAGE_EN,        L"英語 (English)"},
    {IDS_DISPLAY_LANGUAGE_HANLO,     L"漢羅台"},
    {IDS_DISPLAY_LANGUAGE_LO,        L"Lô-jī Tâi"},

    // Input tab
    {IDD_INPUTTAB,                    L"打字"},
    {IDL_INPUTMODE,                   L"打字模式"},
    {IDC_INPUTMODE_CONTINUOUS,        L"自：電腦自動切語詞"},
    {IDC_INPUTMODE_BASIC,             L"揀：我切語詞、電腦鬥揀字"},
    {IDC_INPUTMODE_PRO,               L"手：電腦無鬥相共"},

    {IDL_INPUTMODE_HOTKEY,            L"換打字模式："},
    {IDS_INPUTMODE_KEY_CTRL_PERIOD,   L"ctrl + ."},
    {IDS_INPUTMODE_KEY_CTRL_BACKTICK, L"ctrl + ` (~)"},

    {IDL_ON_OFF_HOTKEY,               L"打字法切掉．点灱："},
    {IDS_ON_OFF_HOTKEY_ALTBACKTICK,   L"alt + ` (~)"},
    {IDS_ON_OFF_HOTKEY_SHIFT,         L"shift"},

    {IDL_TONE_KEYS,                   L"調号："},
    {IDS_TONE_KEYS_NUMERIC,           L"打數字： 2 3 5 7 8 9 0"},
    {IDS_TONE_KEYS_TELEX,             L"Telex： s f l j j w q"},

    {IDL_DOTTED_O_KEY,               L"打「o͘」："},
    {IDS_DOTTED_O_OU,                L"ou"},
    {IDS_DOTTED_O_OO,                L"oo"},
    {IDS_DOTTED_O_Y,                 L"y"},

    {IDL_NASAL_KEY,                  L"打「ⁿ」："},
    {IDS_NASAL_NN,                   L"nn"},
    {IDS_NASAL_V,                    L"v"},
    {IDC_OPTION_UPPERCASE_NASAL,     L"使用大本「ᴺ」"},

    {IDC_OPTION_DOTTED_KHIN,         L"「--」換「·」：打双連劃共換做輕点"},
    {IDC_OPTION_AUTOKHIN,            L"打輕了後、自動共後者變輕"},
    {IDC_OPTION_EASY_CH,             L"「c」換「ch」：打 c 自動加一个 h"},

    {IDL_DEFAULT_PUNCTUATION,        L"標点符号："},
    {IDS_PUNCT_FULL_WIDTH,           L"全 (漢字式 。、！)"},
    {IDS_PUNCT_HALF_WIDTH,           L"半 (羅字式 . , !)"},

    {IDL_HYPHEN_KEY,                 L"連劃「-」："},
    {IDS_HYPHEN_KEY_HYPHEN,          L"-"},
    {IDS_HYPHEN_KEY_V,               L"v"},
};
// clang-format on

} // namespace

std::wstring Strings::T(uint32_t rid, UiLanguage lang) {
    auto ret = std::wstring(L"???");
    StrMap *strs = lang == UiLanguage::HanloTai ? &kStringsHanlo : &kStringsEn;

    if (auto it = strs->find(rid); it != strs->end()) {
        ret = it->second;
    }

    if (ret == L"???") {
        auto x = 3;
    }

    return ret;
}

} // namespace khiin::win32::settings

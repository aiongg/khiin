#include "pch.h"

#include "Strings.h"

#include "Application.h"

namespace khiin::win32::settings {
namespace {
using namespace messages;

using StrMap = std::unordered_map<uint32_t, std::wstring>;

// clang-format off
StrMap kStringsEn = {
    {IDS_WINDOW_CAPTION,         L"Khíín Taiwanese IME Settings"},
    {IDD_APPEARANCETAB,          L"Display"},
    {IDL_COLOR,                  L"Colors:"},
    {IDS_LIGHT_THEME,            L"Light"},
    {IDS_DARK_THEME,             L"Dark"},
    {IDL_CANDIDATE_SIZE,         L"Candidate Font Size:"},
    {IDL_CANDIDATE_SIZE_S,       L"Smaller"},
    {IDL_CANDIDATE_SIZE_L,       L"Larger"},
    {IDL_DISPLAY_LANGUAGE,       L"Display language (介面語言):"},
    {IDS_DISPLAY_LANGUAGE_EN,    L"English"},
    {IDS_DISPLAY_LANGUAGE_HANLO, L"漢羅台 (Hanlo Taiwanese)"},
    {IDS_DISPLAY_LANGUAGE_LO,    L"Lô-jī Tâi (Romanized Taiwanese)"},
    {IDL_EDIT_TRY,               L"Try it:"},
};

StrMap kStringsHanlo = {
    {IDS_WINDOW_CAPTION,         L"起引台語打字法設置"},
    {IDD_APPEARANCETAB,          L"外皮"},
    {IDL_COLOR,                  L"色水："},
    {IDS_LIGHT_THEME,            L"白底"},
    {IDS_DARK_THEME,             L"烏底"},
    {IDL_CANDIDATE_SIZE,         L"揀字大細："},
    {IDL_CANDIDATE_SIZE_S,       L"Khah 細"},
    {IDL_CANDIDATE_SIZE_L,       L"Khah 大"},
    {IDL_EDIT_TRY,               L"打看覓仔："},
    {IDL_DISPLAY_LANGUAGE,       L"介面語言 (Display Language)："},
    {IDS_DISPLAY_LANGUAGE_EN,    L"英語 (English)"},
    {IDS_DISPLAY_LANGUAGE_HANLO, L"漢羅台"},
    {IDS_DISPLAY_LANGUAGE_LO,    L"Lô-jī Tâi"},
};
// clang-format on

} // namespace

std::wstring Strings::T(uint32_t rid, UiLanguage lang) {
    auto ret = std::wstring(L"???");
    StrMap *strs = lang == UIL_TAI_HANLO ? &kStringsHanlo : &kStringsEn;

    if (auto it = strs->find(rid); it != strs->end()) {
        ret = it->second;
    }

    return ret;
}

} // namespace khiin::win32::settings

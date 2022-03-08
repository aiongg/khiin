#include "pch.h"

#include "UiStrings.h"

#include "common.h"

namespace khiin::win32::strings {
using namespace messages;

// clang-format off
std::unordered_map<uint32_t, std::string> kEnglish = {
    {IDS_CONTINUOUS_MODE,           "Continuous"},
    {IDS_BASIC_MODE,                "Basic"},
    {IDS_MANUAL_MODE,               "Manual"},
    {IDS_DIRECT_MODE,               "Alphanumeric (Direct input)"},
    {IDS_OPEN_SETTINGS,             "Khíín Settings"},
};

std::unordered_map<uint32_t, std::string> kHanlo = {
    {IDS_CONTINUOUS_MODE,           "連續打字"},
    {IDS_BASIC_MODE,                "隋个隋个打"},
    {IDS_MANUAL_MODE,               "手打"},
    {IDS_DIRECT_MODE,               "干焦打 ABC (無台語符号)"},
    {IDS_OPEN_SETTINGS,             "起引打字法設置"},
};
// clang-format on

std::string T(uint32_t str_rid, UiLanguage lang) {
    auto map = &kEnglish;
    switch (lang) {
    case UiLanguage::UIL_TAI_HANLO:
        map = &kHanlo;
        break;
    }
    if (auto it = map->find(str_rid); it != map->end()) {
        return it->second;
    }
    return std::string("???");
}

} // namespace khiin::win32::strings
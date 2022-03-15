#include "pch.h"

#include "InputProps.h"

#include "proto/proto.h"
#include "tip/Config.h"

#include "Application.h"
#include "resource.h"

namespace khiin::win32::settings {
namespace {

int index_in(std::vector<uint32_t> const &vec, uint32_t id) {
    auto it = std::find(vec.begin(), vec.end(), id);
    if (it == vec.end()) {
        return -1;
    }
    return static_cast<int>(std::distance(vec.begin(), it));
}

auto kControlResIds = std::vector<uint32_t>{
    // clang-format off
    IDL_INPUTMODE,
    IDC_INPUTMODE_CONTINUOUS,
    IDC_INPUTMODE_BASIC,
    IDC_INPUTMODE_PRO,
    IDL_INPUTMODE_HOTKEY,
    IDL_ON_OFF_HOTKEY,
    IDL_DEFAULT_PUNCTUATION,
    IDL_TONE_KEYS,
    IDL_DOTTED_O_KEY,
    IDL_NASAL_KEY,
    IDC_OPTION_UPPERCASE_NASAL,
    IDL_HYPHEN_KEY,
    IDC_OPTION_DOTTED_KHIN,
    IDC_OPTION_AUTOKHIN,
    IDC_OPTION_EASY_CH,
    // clang-format on
};

auto kOnOffHotkeyIds = std::vector<uint32_t>{
    IDS_ON_OFF_HOTKEY_SHIFT,
    IDS_ON_OFF_HOTKEY_ALTBACKTICK,
};

int SelectedOnOffHotkey() {
    switch (Config::GetOnOffHotkey()) {
    case Hotkey::AltBacktick:
        return index_in(kOnOffHotkeyIds, IDS_ON_OFF_HOTKEY_ALTBACKTICK);
    default:
        return index_in(kOnOffHotkeyIds, IDS_ON_OFF_HOTKEY_SHIFT);
    }
}

void SetOnOffHotkey(int index) {
    if (index < kOnOffHotkeyIds.size()) {
        switch (kOnOffHotkeyIds[index]) {
        case IDS_ON_OFF_HOTKEY_SHIFT:
            Config::SetOnOffHotkey(Hotkey::Shift);
            break;
        case IDS_ON_OFF_HOTKEY_ALTBACKTICK:
            Config::SetOnOffHotkey(Hotkey::AltBacktick);
            break;
        }
    }
}

auto kInputModeHotkeyIds = std::vector<uint32_t>{
    IDS_INPUTMODE_KEY_CTRL_BACKTICK,
    IDS_INPUTMODE_KEY_CTRL_PERIOD,
};

int SelectedInputModeHotkey() {
    switch (Config::GetInputModeHotkey()) {
    case Hotkey::CtrlPeriod:
        return index_in(kInputModeHotkeyIds, IDS_INPUTMODE_KEY_CTRL_PERIOD);
    default:
        return index_in(kInputModeHotkeyIds, IDS_INPUTMODE_KEY_CTRL_BACKTICK);
    }
}

void SetInputModeHotkey(int index) {
    if (index < kInputModeHotkeyIds.size()) {
        switch (kInputModeHotkeyIds[index]) {
        case IDS_INPUTMODE_KEY_CTRL_PERIOD:
            Config::SetInputModeHotkey(Hotkey::CtrlPeriod);
            break;
        case IDS_INPUTMODE_KEY_CTRL_BACKTICK:
            Config::SetInputModeHotkey(Hotkey::CtrlBacktick);
            break;
        }
    }
}

auto kPunctuationTypeIds = std::vector<uint32_t>{
    IDS_PUNCT_FULL_WIDTH,
    IDS_PUNCT_HALF_WIDTH,
};

int SelectedPunctuationType(proto::AppConfig *conf) {
    switch (conf->default_punctuation()) {
    case proto::PUNCT_HALF:
        return index_in(kPunctuationTypeIds, IDS_PUNCT_HALF_WIDTH);
    default:
        return index_in(kPunctuationTypeIds, IDS_PUNCT_FULL_WIDTH);
    }
}

void SetPunctuationType(proto::AppConfig *conf, int index) {
    if (index < kPunctuationTypeIds.size()) {
        switch (kPunctuationTypeIds[index]) {
        case IDS_PUNCT_FULL_WIDTH:
            conf->set_default_punctuation(proto::PUNCT_WHOLE);
            break;
        case IDS_PUNCT_HALF_WIDTH:
            conf->set_default_punctuation(proto::PUNCT_HALF);
            break;
        }
    }
}

auto kToneKeys = std::vector<uint32_t>{
    IDS_TONE_KEYS_NUMERIC,
    IDS_TONE_KEYS_TELEX,
};

int SelectedToneKeys(proto::AppConfig *conf) {
    if (conf->has_telex_enabled() && conf->telex_enabled().value()) {
        return index_in(kToneKeys, IDS_TONE_KEYS_TELEX);
    }
    return index_in(kToneKeys, IDS_TONE_KEYS_NUMERIC);
}

void SetToneKeys(proto::AppConfig *conf, int index) {
    if (index < kToneKeys.size()) {
        switch (kToneKeys[index]) {
        case IDS_TONE_KEYS_NUMERIC:
            conf->mutable_telex_enabled()->set_value(false);
            break;
        case IDS_TONE_KEYS_TELEX:
            conf->mutable_telex_enabled()->set_value(true);
            break;
        }
    }
}

auto kDottedOKeys = std::vector<uint32_t>{
    IDS_DOTTED_O_OU,
    IDS_DOTTED_O_OO,
    IDS_DOTTED_O_Y,
};

int SelectedDottedOKeys(proto::AppConfig *conf) {
    if (conf->has_key_config()) {
        auto &key = conf->key_config().dot_above_right();
        if (key == "y") {
            return index_in(kDottedOKeys, IDS_DOTTED_O_Y);
        } else if (key == "oo") {
            return index_in(kDottedOKeys, IDS_DOTTED_O_OO);
        }
    }
    return index_in(kDottedOKeys, IDS_DOTTED_O_OU);
}

void SetDottedOKeys(proto::AppConfig *conf, int index) {
    if (index < kDottedOKeys.size()) {
        switch (kDottedOKeys[index]) {
        case IDS_DOTTED_O_OU:
            conf->mutable_key_config()->set_dot_above_right("ou");
            break;
        case IDS_DOTTED_O_OO:
            conf->mutable_key_config()->set_dot_above_right("oo");
            break;
        case IDS_DOTTED_O_Y:
            conf->mutable_key_config()->set_dot_above_right("y");
            break;
        }
    }
}

auto kNasalKeys = std::vector<uint32_t>{
    IDS_NASAL_NN,
};

auto kHyphenKeys = std::vector<uint32_t>{
    IDS_HYPHEN_KEY_HYPHEN,
    IDS_HYPHEN_KEY_V,
};

int SelectedHyphenKey(proto::AppConfig *conf) {
    if (conf->has_key_config()) {
        auto &key = conf->key_config().alt_hyphen();
        if (key == "v") {
            return index_in(kHyphenKeys, IDS_HYPHEN_KEY_V);
        }
    }
    return index_in(kHyphenKeys, IDS_HYPHEN_KEY_HYPHEN);
}

void SetHyphenKey(proto::AppConfig* conf, int index) {
    if (index < kHyphenKeys.size()) {
        switch (kHyphenKeys[index]) {
        case IDS_HYPHEN_KEY_HYPHEN:
            conf->mutable_key_config()->mutable_alt_hyphen()->clear();
            break;
        case IDS_HYPHEN_KEY_V:
            conf->mutable_key_config()->set_alt_hyphen("v");
            break;
        }
    }
}

} // namespace

InputProps::InputProps(Application *app) : PropSheetPage(app) {
    m_string_ids = kControlResIds;
}

void InputProps::Initialize() {
    switch (m_config->input_mode()) {
    case proto::IM_CONTINUOUS:
        Button_SetCheck(ItemById(IDC_INPUTMODE_CONTINUOUS), BST_CHECKED);
        break;
    case proto::IM_BASIC:
        Button_SetCheck(ItemById(IDC_INPUTMODE_BASIC), BST_CHECKED);
        break;
    case proto::IM_PRO:
        Button_SetCheck(ItemById(IDC_INPUTMODE_PRO), BST_CHECKED);
        break;
    }

    InitComboBox(IDC_ON_OFF_HOTKEY_COMBO, kOnOffHotkeyIds, SelectedOnOffHotkey());
    InitComboBox(IDC_INPUTMODE_KEY_COMBO, kInputModeHotkeyIds, SelectedInputModeHotkey());
    InitComboBox(IDC_PUNCTUATION_COMBO, kPunctuationTypeIds, SelectedPunctuationType(m_config));
    InitComboBox(IDC_TONE_KEYS_COMBO, kToneKeys, SelectedToneKeys(m_config));
    InitComboBox(IDC_DOTTED_O_KEY_COMBO, kDottedOKeys, SelectedDottedOKeys(m_config));
    InitComboBox(IDC_NASAL_KEY_COMBO, kNasalKeys, 0);
    InitComboBox(IDC_HYPHEN_KEY_COMBO, kHyphenKeys, SelectedHyphenKey(m_config));

    Button_SetCheck(ItemById(IDC_OPTION_DOTTED_KHIN), m_config->dotted_khin().value() ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(ItemById(IDC_OPTION_AUTOKHIN), m_config->autokhin().value() ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(ItemById(IDC_OPTION_EASY_CH), m_config->easy_ch().value() ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(ItemById(IDC_OPTION_UPPERCASE_NASAL), m_config->uppercase_nasal().value() ? BST_CHECKED : BST_UNCHECKED);

    PropSheetPage::Initialize();
}

void InputProps::Finalize() {
    HWND item = NULL;

    if (Button_GetCheck(ItemById(IDC_INPUTMODE_CONTINUOUS)) == BST_CHECKED) {
        m_config->set_input_mode(proto::IM_CONTINUOUS);
    } else if (Button_GetCheck(ItemById(IDC_INPUTMODE_BASIC)) == BST_CHECKED) {
        m_config->set_input_mode(proto::IM_BASIC);
    } else if (Button_GetCheck(ItemById(IDC_INPUTMODE_PRO)) == BST_CHECKED) {
        m_config->set_input_mode(proto::IM_PRO);
    }

    SetOnOffHotkey(ComboBox_GetCurSel(ItemById(IDC_ON_OFF_HOTKEY_COMBO)));
    SetInputModeHotkey(ComboBox_GetCurSel(ItemById(IDC_INPUTMODE_KEY_COMBO)));
    SetPunctuationType(m_config, ComboBox_GetCurSel(ItemById(IDC_PUNCTUATION_COMBO)));
    SetToneKeys(m_config, ComboBox_GetCurSel(ItemById(IDC_TONE_KEYS_COMBO)));
    SetDottedOKeys(m_config, ComboBox_GetCurSel(ItemById(IDC_DOTTED_O_KEY_COMBO)));
    SetHyphenKey(m_config, ComboBox_GetCurSel(ItemById(IDC_HYPHEN_KEY_COMBO)));
    m_config->mutable_dotted_khin()->set_value(Button_GetCheck(ItemById(IDC_OPTION_DOTTED_KHIN)) == BST_CHECKED);
    m_config->mutable_autokhin()->set_value(Button_GetCheck(ItemById(IDC_OPTION_AUTOKHIN)) == BST_CHECKED);
    m_config->mutable_easy_ch()->set_value(Button_GetCheck(ItemById(IDC_OPTION_EASY_CH)) == BST_CHECKED);
    m_config->mutable_uppercase_nasal()->set_value(Button_GetCheck(ItemById(IDC_OPTION_UPPERCASE_NASAL)) ==
                                                   BST_CHECKED);
}

} // namespace khiin::win32::settings

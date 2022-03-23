#include "pch.h"

#include "PreservedKeyMgr.h"

#include "TextService.h"

#include "Config.h"
#include "Guids.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

struct PreservedKey {
    GUID guid;
    TF_PRESERVEDKEY prekey;
    std::wstring desc;
};

inline constexpr int kVkBacktick = VK_OEM_3;

// VK_OEM_3 = ` (BACK QUOTE)
PreservedKey kPrekeyOnOff = {guids::kPreservedKeyOnOff, {kVkBacktick, MOD_ALT}, L"Direct input"};
PreservedKey kPreKeySwitchMode = {guids::kPreservedKeySwitchMode, {kVkBacktick, MOD_CONTROL}, L"Switch Mode"};
PreservedKey kPrekeyFWS = {guids::kPreservedKeyFullWidthSpace, {VK_SPACE, MOD_SHIFT}, L"Full width space"};

class PreservedKeyMgrImpl : public PreservedKeyMgr, ConfigChangeListener {
    void Initialize(TextService *service) override {
        m_service.copy_from(service);
        m_service->RegisterConfigChangeListener(this);
        m_switch_mode_key = kPreKeySwitchMode;
    }

    void OnConfigChanged(proto::AppConfig *config) override {
        UnpreserveKeys();
        PreserveKeys();
    }

    void Shutdown() override {
        auto keystroke_mgr = KeystrokeMgr();
        UnpreserveKeys();
        m_service = nullptr;
    }

    com_ptr<ITfKeystrokeMgr> KeystrokeMgr() {
        auto thread_mgr = m_service->thread_mgr();
        return thread_mgr.as<ITfKeystrokeMgr>();
    }

    void PreserveKeys() {
        if (Config::GetOnOffHotkey() == Hotkey::AltBacktick) {
            PreserveKey(kPrekeyOnOff);
        } else {
            UnpreserveKey(kPrekeyOnOff);
        }

        switch (Config::GetInputModeHotkey()) {
        case Hotkey::CtrlPeriod:
            m_switch_mode_key.prekey = {VK_OEM_PERIOD, MOD_CONTROL};
            break;
        case Hotkey::CtrlBacktick:
            m_switch_mode_key.prekey = {kVkBacktick, MOD_CONTROL};
            break;
        }

        PreserveKey(m_switch_mode_key);
    }

    void PreserveKey(PreservedKey const &pk) {
        auto hr = KeystrokeMgr()->PreserveKey(m_service->client_id(), pk.guid, &pk.prekey, pk.desc.data(),
                                    static_cast<ULONG>(pk.desc.size()));
        
        CHECK_HRESULT(hr);
    }

    void UnpreserveKeys() {
        UnpreserveKey(kPrekeyOnOff);
        UnpreserveKey(kPrekeyFWS);
        UnpreserveKey(m_switch_mode_key);
    }

    void UnpreserveKey(PreservedKey const &pk) {
        KeystrokeMgr()->UnpreserveKey(pk.guid, &pk.prekey);
    }

    com_ptr<TextService> m_service;
    PreservedKey m_switch_mode_key;
};

} // namespace

std::unique_ptr<PreservedKeyMgr> PreservedKeyMgr::Create() {
    return std::make_unique<PreservedKeyMgrImpl>();
}

} // namespace khiin::win32::tip

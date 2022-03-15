#include "pch.h"

#include "PreservedKeyMgr.h"

#include "TextService.h"

#include "Config.h"
#include "Guids.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

struct PreservedKey {
    const GUID &guid;
    const TF_PRESERVEDKEY prekey;
    const std::wstring desc;
};

// VK_OEM_3 = ` (BACK QUOTE)
PreservedKey kPrekeyOnOff = {guids::kPreservedKeyOnOff, {VK_OEM_3, MOD_ALT}, L"Direct input"};
PreservedKey kPrekeyFWS = {guids::kPreservedKeyFullWidthSpace, {VK_SPACE, MOD_SHIFT}, L"Full width space"};

class PreservedKeyMgrImpl : public PreservedKeyMgr, ConfigChangeListener {
    virtual void Initialize(TextService *service) override {
        m_service.copy_from(service);
        m_service->RegisterConfigChangeListener(this);
    }

    virtual void OnConfigChanged(proto::AppConfig *config) override {
        PreserveKeys();
    }

    virtual void Shutdown() override {
        auto keystroke_mgr = KeystrokeMgr();
        UnpreserveKey(kPrekeyOnOff);
        UnpreserveKey(kPrekeyFWS);
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
    }

    void PreserveKey(PreservedKey const &pk) {
        KeystrokeMgr()->PreserveKey(m_service->clientId(), pk.guid, &pk.prekey, pk.desc.data(),
                                    static_cast<ULONG>(pk.desc.size()));
    }

    void UnpreserveKey(PreservedKey const &pk) {
        KeystrokeMgr()->UnpreserveKey(pk.guid, &pk.prekey);
    }

    com_ptr<TextService> m_service;
};

} // namespace

std::unique_ptr<PreservedKeyMgr> PreservedKeyMgr::Create() {
    return std::make_unique<PreservedKeyMgrImpl>();
}

} // namespace khiin::win32::tip

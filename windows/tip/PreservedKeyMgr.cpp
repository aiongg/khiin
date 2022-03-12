#include "pch.h"

#include "PreservedKeyMgr.h"

#include "TextService.h"

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

class PreservedKeyMgrImpl : public PreservedKeyMgr {
    virtual void Initialize(TextService *service) override {
        m_service.copy_from(service);
        auto client_id = m_service->clientId();
        auto keystroke_mgr = KeystrokeMgr();
        PreserveKey(client_id, keystroke_mgr.get(), kPrekeyOnOff);
        PreserveKey(client_id, keystroke_mgr.get(), kPrekeyFWS);
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

    void PreserveKey(TfClientId tid, ITfKeystrokeMgr *keystroke_mgr, PreservedKey const &pk) {
        keystroke_mgr->PreserveKey(tid, pk.guid, &pk.prekey, pk.desc.data(), static_cast<ULONG>(pk.desc.size()));
    }

    void UnpreserveKey(PreservedKey const &pk) {}

    com_ptr<TextService> m_service;
};

} // namespace

std::unique_ptr<PreservedKeyMgr> PreservedKeyMgr::Create() {
    return std::make_unique<PreservedKeyMgrImpl>();
}

} // namespace khiin::win32::tip

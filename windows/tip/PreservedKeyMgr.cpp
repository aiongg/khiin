#include "pch.h"

#include "PreservedKeyMgr.h"

#include "TextService.h"

#include "Guids.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

class PreservedKeyMgrImpl : public PreservedKeyMgr {
    virtual void Initialize(TextService *service) override {
        m_service.copy_from(service);
    }

    virtual void Shutdown() override {}
    
    com_ptr<TextService> m_service;
};

} // namespace

PreservedKeyMgr *PreservedKeyMgr::Create() {
    return new PreservedKeyMgrImpl();
}

} // namespace khiin::win32::tip
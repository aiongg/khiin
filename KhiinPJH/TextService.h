#pragma once

#include "pch.h"

namespace Khiin {

// clang-format off
struct TextService : winrt::implements<TextService,
                                       ITfTextInputProcessorEx,
                                       ITfDisplayAttributeProvider,
                                       ITfThreadFocusSink,
                                       ITfTextLayoutSink,
                                       ITfCompartmentEventSink> { // clang-format on

    TfClientId clientId();
    ITfThreadMgr *threadMgr();
    DWORD getActivateFlags();
    IEnumTfDisplayAttributeInfo *displayAttrInfoEnum();

    // ITfTextInputProcessorEx
    virtual STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid) override;
    virtual STDMETHODIMP Deactivate(void) override;
    virtual STDMETHODIMP ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags) override;

    // ITfDisplayAttributeProvider
    virtual STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) override;
    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override;

    // ITfThreadFocusSink
    virtual STDMETHODIMP OnSetThreadFocus(void) override;
    virtual STDMETHODIMP OnKillThreadFocus(void) override;

    // ITfTextLayoutSink
    virtual STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView) override;

    // ITfCompartmentEventSink
    virtual STDMETHODIMP OnChange(REFGUID rguid) override;

  private:
    winrt::com_ptr<ITfThreadMgr> threadMgr_ = nullptr;
    TfClientId clientId_ = TF_CLIENTID_NULL;
    DWORD dwActivateFlags = 0;

    DEFAULT_CTOR_DTOR(TextService);
    DELETE_COPY_AND_ASSIGN(TextService);
};

} // namespace Khiin

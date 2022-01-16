#pragma once

#include "pch.h"

#include "Compartment.h"
#include "CompositionMgr.h"
#include "KeyEventSink.h"
#include "TextEngine.h"
#include "ThreadMgrEventSink.h"
#include "DisplayAttributeInfoEnum.h"

namespace Khiin {

struct TextService : winrt::implements<TextService, ITfTextInputProcessorEx, ITfDisplayAttributeProvider,
                                       ITfThreadFocusSink, ITfTextLayoutSink, ITfCompartmentEventSink> {
    TextService() = default;
    TextService(const TextService &) = delete;
    TextService &operator=(const TextService &) = delete;
    ~TextService() = default;

    // Inherited via implements
    virtual STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid) override;
    virtual STDMETHODIMP Deactivate(void) override;
    virtual STDMETHODIMP ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags) override;
    virtual STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum) override;
    virtual STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo) override;
    virtual STDMETHODIMP OnSetThreadFocus(void) override;
    virtual STDMETHODIMP OnKillThreadFocus(void) override;
    virtual STDMETHODIMP OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pView) override;
    virtual STDMETHODIMP OnChange(REFGUID rguid) override;

  private:
    HRESULT getTopContext(_Out_ ITfContext **pContext);
    HRESULT setOpenClose(bool openclose);

    winrt::com_ptr<TextEngine> engine = nullptr;
    // winrt::com_ptr<TextEditSink> textEditSink = nullptr;
    winrt::com_ptr<ITfThreadMgr> threadMgr = nullptr;
    TfClientId clientId = TF_CLIENTID_NULL;
    DWORD dwActivateFlags = 0;

    winrt::com_ptr<DisplayAttributeInfoEnum> attributes = nullptr;
    winrt::com_ptr<ThreadMgrEventSink> threadMgrEventSink = nullptr;
    winrt::com_ptr<KeyEventSink> keyEventSink = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr = nullptr;

    Compartment openCloseCompartment;
    Compartment keyboardDisabledCompartment;
    SinkManager<ITfCompartmentEventSink> openCloseSinkInstaller;

    DWORD openCloseSinkCookie = TF_INVALID_COOKIE;
};

} // namespace Khiin

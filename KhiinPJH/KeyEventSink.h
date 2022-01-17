#pragma once

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "TextEngine.h"

namespace Khiin {

struct KeyEventSink : winrt::implements<KeyEventSink, ITfKeyEventSink> {
  private:
    enum class KeyAction;

  public:
    HRESULT init(_In_ TfClientId clientId, _In_ ITfThreadMgr *pThreadMgr, _In_ CompositionMgr *pCompositionMgr,
                 _In_ CandidateListUI *pCandidateListUI, _In_ TextEngine *pEngine);
    HRESULT uninit();

    HRESULT onTestKey(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    HRESULT onKey(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);

    // ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten) override;

  private:
    TfClientId clientId = TF_CLIENTID_NULL;
    winrt::com_ptr<CandidateListUI> candidateListUI = nullptr;
    winrt::com_ptr<TextEngine> engine = nullptr;
    winrt::com_ptr<ITfThreadMgr> threadMgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> keystrokeMgr = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr = nullptr;

    DEFAULT_CTOR_DTOR(KeyEventSink);
    DELETE_COPY_AND_ASSIGN(KeyEventSink);
};

} // namespace Khiin

#pragma once

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "ITextEngine.h"
#include "ITextService.h"
#include "KeyEvent.h"

namespace Khiin {

struct KeyEventSink : winrt::implements<KeyEventSink, ITfKeyEventSink> {
  private:
    enum class KeyAction;

  public:
    KeyEventSink() = default;
    ~KeyEventSink();
    HRESULT init(ITextService *pTextService);
    HRESULT uninit();

    HRESULT onTestKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten);
    HRESULT onKey(ITfContext *pContext, KeyEvent keyEvent, BOOL *pfEaten);

    // ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten) override;

  private:
    winrt::com_ptr<ITextService> service = nullptr;
    //winrt::com_ptr<CandidateListUI> candidateListUI = nullptr;
    //winrt::com_ptr<ITextEngine> engine = nullptr;
    winrt::com_ptr<ITfThreadMgr> threadMgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> keystrokeMgr = nullptr;
    winrt::com_ptr<CompositionMgr> compositionMgr = nullptr;

    DELETE_COPY_AND_ASSIGN(KeyEventSink);
};

} // namespace Khiin

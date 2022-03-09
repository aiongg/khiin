#pragma once

#include "CandidateListUI.h"
#include "CompositionMgr.h"
#include "TextService.h"
#include "KeyEvent.h"

namespace khiin::win32::tip {

struct KeyEventSink : winrt::implements<KeyEventSink, ITfKeyEventSink> {
  private:
    enum class KeyAction;

  public:
    KeyEventSink() = default;
    KeyEventSink(const KeyEventSink &) = delete;
    KeyEventSink &operator=(const KeyEventSink &) = delete;
    ~KeyEventSink();
    void Activate(TextService *pTextService);
    void Deactivate();

    void TestKey(ITfContext *pContext, win32::KeyEvent keyEvent, BOOL *pfEaten);
    void HandleKey(ITfContext *pContext, win32::KeyEvent keyEvent, BOOL *pfEaten);

    // ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten) override;

  private:
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfThreadMgr> thread_mgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> keystrokeMgr = nullptr;
    winrt::com_ptr<CompositionMgr> composition_mgr = nullptr;
};

} // namespace khiin::win32

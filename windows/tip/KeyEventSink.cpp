#include "pch.h"

#include "KeyEventSink.h"

#include "proto/proto.h"

#include "EditSession.h"
#include "EngineController.h"
#include "Guids.h"
#include "KeyEvent.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

bool TestPageKeyForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    if (cand_ui->Selecting()) {
        auto key = key_event.keyCode();
        if (cand_ui->MultiColumn() && (key == VK_LEFT || key == VK_RIGHT)) {
            return true;
        }

        if (cand_ui->PageCount() > 1 && (key == VK_NEXT || key == VK_PRIOR)) {
            return true;
        }
    }

    return false;
}

bool TestQuickSelectForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    if (cand_ui->Selecting() && key_event.ascii()) {
        auto qs = key_event.ascii() - '0';
        auto qs_max = cand_ui->MaxQuickSelect();
        return 1 <= qs && qs <= qs_max;
    }

    return false;
}

bool TestKeyForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    return TestPageKeyForCandidateUI(cand_ui, key_event) || TestQuickSelectForCandidateUI(cand_ui, key_event);
}

bool HandleQuickSelect(TextService *service, ITfContext *context, CandidateListUI *cand_ui,
                       win32::KeyEvent const &key_event) {
    auto id = cand_ui->QuickSelect(key_event.ascii() - '0' - 1);

    if (id >= 0) {
        auto command = service->engine()->SelectCandidate(id);
        EditSession::HandleAction(service, context, command);
        return true;
    }

    return false;
}

bool HandleCandidatePage(TextService *service, ITfContext *context, CandidateListUI *cand_ui,
                         win32::KeyEvent const &key_event) {

    auto id = -1;

    if (key_event.keyCode() == VK_NEXT) {
        id = cand_ui->RotateNext();
    } else if (key_event.keyCode() == VK_PRIOR) {
        id = cand_ui->RotatePrev();
    }

    if (id >= 0) {
        auto command = service->engine()->FocusCandidate(id);
        EditSession::HandleAction(service, context, std::move(command));
        return true;
    }

    // TODO: Handle left/right in multi-column grid

    return false;
}

void HandleKeyBasic(TextService *service, ITfContext *context, win32::KeyEvent const &key_event) {
    auto command = service->engine()->OnKey(key_event);
    EditSession::HandleAction(service, context, std::move(command));
}

struct KeyEventSinkImpl : implements<KeyEventSinkImpl, ITfKeyEventSink, KeyEventSink> {

    virtual void Advise(TextService *pTextService) override {
        service.copy_from(pTextService);
        composition_mgr.copy_from(cast_as<CompositionMgr>(service->composition_mgr()));
        keystrokeMgr = service->keystroke_mgr();

        winrt::check_hresult(keystrokeMgr->AdviseKeyEventSink(service->clientId(), this, TRUE));
    }

    virtual void Unadvise() override {
        KHIIN_TRACE("");
        if (keystrokeMgr) {
            winrt::check_hresult(keystrokeMgr->UnadviseKeyEventSink(service->clientId()));
        }

        keystrokeMgr = nullptr;
        composition_mgr = nullptr;
        service = nullptr;
    }

    void TestKey(ITfContext *pContext, win32::KeyEvent keyEvent, BOOL *pfEaten) {
        KHIIN_TRACE("");
        WINRT_ASSERT(pContext);
        WINRT_ASSERT(composition_mgr);

        if (TestKeyForCandidateUI(service->candidate_ui().get(), keyEvent)) {
            *pfEaten = TRUE;
            return;
        }

        if (!composition_mgr->composing()) {
            service->engine()->Reset();
        }

        auto command = service->engine()->TestKey(keyEvent);

        if (command->response().consumable()) {
            *pfEaten = TRUE;
            return;
        }

        EditSession::HandleAction(service.get(), pContext, std::move(command));
    }

    void HandleKey(ITfContext *pContext, win32::KeyEvent keyEvent, BOOL *pfEaten) {
        auto cui = service->candidate_ui().get();

        if (TestQuickSelectForCandidateUI(cui, keyEvent)) {
            HandleQuickSelect(service.get(), pContext, cui, keyEvent);
            *pfEaten = TRUE;
            return;
        } else if (TestPageKeyForCandidateUI(cui, keyEvent)) {
            HandleCandidatePage(service.get(), pContext, cui, keyEvent);
            *pfEaten = TRUE;
            return;
        }

        TestKey(pContext, keyEvent, pfEaten);

        if (!*pfEaten) {
            return;
        }

        HandleKeyBasic(service.get(), pContext, keyEvent);
    }

    //+---------------------------------------------------------------------------
    //
    // ITfKeyEventSink
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override {
        TRY_FOR_HRESULT;
        if (!fForeground) {
            return S_OK;
        }

        return E_NOTIMPL;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");

        *pfEaten = false;
        auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wParam, lParam);
        TestKey(pContext, keyEvent, pfEaten);

        return S_OK;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        auto keyEvent = win32::KeyEvent(WM_KEYUP, wParam, lParam);
        KHIIN_DEBUG("OnTestKeyUp");

        return E_NOTIMPL;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;
        KHIIN_TRACE("");

        *pfEaten = false;
        auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wParam, lParam);
        HandleKey(pContext, keyEvent, pfEaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;
        auto keyEvent = win32::KeyEvent(WM_KEYUP, wParam, lParam);

        return E_NOTIMPL;
        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        if (rguid == guids::kPreservedKeyOnOff) {
            service->SwapOnOff();
            *pfEaten = TRUE;
        }
        CATCH_FOR_HRESULT;
    }

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfThreadMgr> thread_mgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> keystrokeMgr = nullptr;
    winrt::com_ptr<CompositionMgr> composition_mgr = nullptr;
};

} // namespace

com_ptr<KeyEventSink> KeyEventSink::Create() {
    return as_self<KeyEventSink>(make_self<KeyEventSinkImpl>());
}

} // namespace khiin::win32::tip

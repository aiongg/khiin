#include "pch.h"

#include "KeyEventSink.h"

#include "proto/proto.h"

#include "Config.h"
#include "EditSession.h"
#include "EngineController.h"
#include "Guids.h"
#include "KeyEvent.h"

namespace khiin::win32::tip {
namespace {
using namespace winrt;

inline bool ShiftOnOff() {
    return Config::GetOnOffHotkey() == Hotkey::Shift;
}

bool TestPageKeyForCandidateUI(CandidateListUI *cand_ui, win32::KeyEvent const &key_event) {
    if (cand_ui->Selecting()) {
        auto key = key_event.keycode();
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

    if (key_event.keycode() == VK_NEXT) {
        id = cand_ui->RotateNext();
    } else if (key_event.keycode() == VK_PRIOR) {
        id = cand_ui->RotatePrev();
    }

    if (id >= 0) {
        auto command = service->engine()->FocusCandidate(id);
        EditSession::HandleAction(service, context, command);
        return true;
    }

    // TODO: Handle left/right in multi-column grid

    return false;
}

void HandleKeyBasic(TextService *service, ITfContext *context, win32::KeyEvent const &key_event) {
    auto command = service->engine()->OnKey(key_event);
    EditSession::HandleAction(service, context, command);
}

struct KeyEventSinkImpl : implements<KeyEventSinkImpl, ITfKeyEventSink, KeyEventSink> {

    void Advise(TextService *pTextService) override {
        service.copy_from(pTextService);
        keystrokeMgr = service->keystroke_mgr();

        winrt::check_hresult(keystrokeMgr->AdviseKeyEventSink(service->clientId(), this, TRUE));
    }

    void Unadvise() override {
        KHIIN_TRACE("");
        if (keystrokeMgr) {
            winrt::check_hresult(keystrokeMgr->UnadviseKeyEventSink(service->clientId()));
        }

        keystrokeMgr = nullptr;
        service = nullptr;
    }

    void Reset() override {
        shift_pressed = false;
    }

    void TestKey(ITfContext *context, win32::KeyEvent key_event, BOOL *pfEaten) {
        KHIIN_TRACE("");
        WINRT_ASSERT(context);
        KHIIN_DEBUG("TestKey: {}", std::string(1, key_event.ascii()));

        auto changed = service->OnContextChange(context);
        KHIIN_DEBUG("Context changed? {}", changed);

        if (!service->Enabled()) {
            *pfEaten = FALSE;
            return;
        }

        shift_pressed = false;

        if (ShiftOnOff() && key_event.keycode() == VK_SHIFT) {
            shift_pressed = true;
            *pfEaten = TRUE;
            return;
        }

        if (TestKeyForCandidateUI(service->candidate_ui().get(), key_event)) {
            *pfEaten = TRUE;
            return;
        }

        if (!service->composition_mgr()->composing()) {
            service->engine()->Reset();
        }

        auto command = service->engine()->TestKey(key_event);

        if (command->response().consumable()) {
            KHIIN_DEBUG("Key is consumable");
            *pfEaten = TRUE;
            return;
        }

        KHIIN_DEBUG("Key is not consumable");
        *pfEaten = FALSE;
        //EditSession::HandleAction(service.get(), context, command);
    }

    void HandleKey(ITfContext *pContext, win32::KeyEvent key_event, BOOL *pfEaten) {
        KHIIN_DEBUG("HandleKey: {}", std::string(1, key_event.ascii()));
        auto cui = service->candidate_ui().get();

        if (!service->Enabled()) {
            *pfEaten = FALSE;
            return;
        }

        if (TestQuickSelectForCandidateUI(cui, key_event)) {
            HandleQuickSelect(service.get(), pContext, cui, key_event);
            *pfEaten = TRUE;
            return;
        } else if (TestPageKeyForCandidateUI(cui, key_event)) {
            HandleCandidatePage(service.get(), pContext, cui, key_event);
            *pfEaten = TRUE;
            return;
        }

        TestKey(pContext, key_event, pfEaten);

        if (*pfEaten == 0 || shift_pressed) {
            return;
        }

        shift_pressed = false;

        HandleKeyBasic(service.get(), pContext, key_event);
    }

    void TestKeyUp(ITfContext *context, win32::KeyEvent key_event, BOOL *pfEaten) {
        KHIIN_DEBUG("TestKeyUp: {}", std::string(1, key_event.ascii()));
        if (ShiftOnOff() && shift_pressed && key_event.keycode() == VK_SHIFT) {
            *pfEaten = TRUE;
        }
    }

    void HandleKeyUp(ITfContext *context, win32::KeyEvent key_event, BOOL *pfEaten) {
        KHIIN_DEBUG("HandleKeyUp: {}", std::string(1, key_event.ascii()));
        if (ShiftOnOff() && shift_pressed && key_event.keycode() == VK_SHIFT) {
            service->TipOnOff();
            *pfEaten = TRUE;
        }
    }

    //+---------------------------------------------------------------------------
    //
    // ITfKeyEventSink
    //
    //----------------------------------------------------------------------------

    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override {
        KHIIN_DEBUG("OnSetFocus");
        return S_OK;
    }

    virtual STDMETHODIMP OnTestKeyDown(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        *pfEaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wParam, lParam);
        TestKey(context, keyEvent, pfEaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnTestKeyUp(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        *pfEaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYUP, wParam, lParam);
        TestKeyUp(context, keyEvent, pfEaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnKeyDown(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        *pfEaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wParam, lParam);
        HandleKey(context, keyEvent, pfEaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnKeyUp(ITfContext *context, WPARAM wParam, LPARAM lParam, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        *pfEaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYUP, wParam, lParam);
        HandleKeyUp(context, keyEvent, pfEaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnPreservedKey(ITfContext *context, REFGUID rguid, BOOL *pfEaten) override {
        TRY_FOR_HRESULT;

        if (rguid == guids::kPreservedKeyOnOff) {
            service->TipOnOff();
            *pfEaten = TRUE;
        }

        if (rguid == guids::kPreservedKeySwitchMode) {
            service->CycleInputMode();
            *pfEaten = TRUE;
        }

        if (rguid == guids::kPreservedKeyFullWidthSpace) {
        }

        CATCH_FOR_HRESULT;
    }

    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfThreadMgr> thread_mgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> keystrokeMgr = nullptr;
    bool shift_pressed = false;
};

} // namespace

com_ptr<KeyEventSink> KeyEventSink::Create() {
    return as_self<KeyEventSink>(make_self<KeyEventSinkImpl>());
}

} // namespace khiin::win32::tip

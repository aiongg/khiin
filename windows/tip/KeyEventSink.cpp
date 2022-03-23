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

    void Advise(TextService *service) override {
        m_service.copy_from(service);
        m_keystroke_mgr = m_service->keystroke_mgr();

        winrt::check_hresult(m_keystroke_mgr->AdviseKeyEventSink(m_service->client_id(), this, TRUE));
    }

    void Unadvise() override {
        KHIIN_TRACE("");
        if (m_keystroke_mgr) {
            winrt::check_hresult(m_keystroke_mgr->UnadviseKeyEventSink(m_service->client_id()));
        }

        m_keystroke_mgr = nullptr;
        m_service = nullptr;
    }

    void Reset() override {
        m_shift_pressed = false;
    }

    void TestKey(ITfContext *context, win32::KeyEvent key_event, BOOL *pfEaten) {
        KHIIN_TRACE("");
        WINRT_ASSERT(context);
        KHIIN_DEBUG("TestKey: {}", std::string(1, key_event.ascii()));

        auto changed = m_service->UpdateContext(context);
        KHIIN_DEBUG("Context changed? {}", changed);

        if (!m_service->Enabled()) {
            *pfEaten = FALSE;
            return;
        }

        m_shift_pressed = false;

        if (ShiftOnOff() && key_event.keycode() == VK_SHIFT) {
            m_shift_pressed = true;
            *pfEaten = TRUE;
            return;
        }

        if (TestKeyForCandidateUI(m_service->candidate_ui().get(), key_event)) {
            *pfEaten = TRUE;
            return;
        }

        if (!m_service->composition_mgr()->composing()) {
            m_service->engine()->Reset();
        }

        auto command = m_service->engine()->TestKey(key_event);

        if (command->response().consumable()) {
            KHIIN_DEBUG("Key is consumable");
            *pfEaten = TRUE;
            return;
        }

        KHIIN_DEBUG("Key is not consumable");
        *pfEaten = FALSE;
        //EditSession::HandleAction(m_service.get(), context, command);
    }

    void HandleKey(ITfContext *context, win32::KeyEvent key_event, BOOL *eaten) {
        KHIIN_DEBUG("HandleKey: {}", std::string(1, key_event.ascii()));
        auto cui = m_service->candidate_ui().get();

        if (!m_service->Enabled()) {
            *eaten = FALSE;
            return;
        }

        if (TestQuickSelectForCandidateUI(cui, key_event)) {
            HandleQuickSelect(m_service.get(), context, cui, key_event);
            *eaten = TRUE;
            return;
        } else if (TestPageKeyForCandidateUI(cui, key_event)) {
            HandleCandidatePage(m_service.get(), context, cui, key_event);
            *eaten = TRUE;
            return;
        }

        TestKey(context, key_event, eaten);

        if (*eaten == 0 || m_shift_pressed) {
            return;
        }

        m_shift_pressed = false;

        HandleKeyBasic(m_service.get(), context, key_event);
    }

    void TestKeyUp(ITfContext *context, win32::KeyEvent key_event, BOOL *eaten) {
        KHIIN_DEBUG("TestKeyUp: {}", std::string(1, key_event.ascii()));
        if (ShiftOnOff() && m_shift_pressed && key_event.keycode() == VK_SHIFT) {
            *eaten = TRUE;
        }
    }

    void HandleKeyUp(ITfContext *context, win32::KeyEvent key_event, BOOL *eaten) {
        KHIIN_DEBUG("HandleKeyUp: {}", std::string(1, key_event.ascii()));
        if (ShiftOnOff() && m_shift_pressed && key_event.keycode() == VK_SHIFT) {
            m_service->TipOnOff();
            *eaten = TRUE;
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

    virtual STDMETHODIMP OnTestKeyDown(ITfContext *context, WPARAM wparam, LPARAM lparam, BOOL *eaten) override {
        TRY_FOR_HRESULT;

        *eaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wparam, lparam);
        TestKey(context, keyEvent, eaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnTestKeyUp(ITfContext *context, WPARAM wparam, LPARAM lparam, BOOL *eaten) override {
        TRY_FOR_HRESULT;

        *eaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYUP, wparam, lparam);
        TestKeyUp(context, keyEvent, eaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnKeyDown(ITfContext *context, WPARAM wparam, LPARAM lparam, BOOL *eaten) override {
        TRY_FOR_HRESULT;

        *eaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYDOWN, wparam, lparam);
        HandleKey(context, keyEvent, eaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnKeyUp(ITfContext *context, WPARAM wparam, LPARAM lparam, BOOL *eaten) override {
        TRY_FOR_HRESULT;

        *eaten = FALSE;
        auto keyEvent = win32::KeyEvent(WM_KEYUP, wparam, lparam);
        HandleKeyUp(context, keyEvent, eaten);

        CATCH_FOR_HRESULT;
    }

    virtual STDMETHODIMP OnPreservedKey(ITfContext *context, REFGUID rguid, BOOL *eaten) override {
        TRY_FOR_HRESULT;

        if (rguid == guids::kPreservedKeyOnOff) {
            m_service->TipOnOff();
            *eaten = TRUE;
        }

        if (rguid == guids::kPreservedKeySwitchMode) {
            m_service->CycleInputMode();
            *eaten = TRUE;
        }

        if (rguid == guids::kPreservedKeyFullWidthSpace) {
        }

        CATCH_FOR_HRESULT;
    }

    winrt::com_ptr<TextService> m_service = nullptr;
    winrt::com_ptr<ITfThreadMgr> thread_mgr = nullptr;
    winrt::com_ptr<ITfKeystrokeMgr> m_keystroke_mgr = nullptr;
    bool m_shift_pressed = false;
};

} // namespace

com_ptr<KeyEventSink> KeyEventSink::Create() {
    return as_self<KeyEventSink>(make_self<KeyEventSinkImpl>());
}

} // namespace khiin::win32::tip

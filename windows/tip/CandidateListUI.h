#pragma once

#include "common.h"

namespace khiin::win32 {

class KeyEvent;
struct TextService;

struct CandidateListUI : winrt::implements<CandidateListUI, IUnknown> {
    CandidateListUI() = default;
    CandidateListUI(const CandidateListUI &) = delete;
    CandidateListUI &operator=(const CandidateListUI &) = delete;
    ~CandidateListUI() = default;

    virtual void Initialize(TextService *pTextService) = 0;
    virtual void Uninitialize() = 0;
    virtual void DestroyCandidateWindow() = 0;
    virtual void Update(ITfContext *pContext, messages::EditState edit_state,
                        const messages::CandidateList &candidate_list, RECT text_rect) = 0;

    virtual bool Showing() = 0;
    virtual bool Selecting() = 0;
    virtual bool MultiColumn() = 0;
    virtual int PageCount() = 0;
    virtual int MaxQuickSelect() = 0;

    // Returns the ID of the selected candidate
    virtual int QuickSelect(int index) = 0;

    // Returns the ID of the focused candidate after rotating
    // one column or page to the right
    virtual int RotateNext() = 0;

    // Returns the ID of the focused candidate after rotating
    // one column or page to the left
    virtual int RotatePrev() = 0;

    virtual void Show() = 0;
    virtual void Hide() = 0;

    virtual void OnSetThreadFocus() = 0;
    virtual void OnKillThreadFocus() = 0;
    virtual ITfContext *context() = 0;
};

struct CandidateListUIFactory {
    static HRESULT Create(CandidateListUI **ppCandidateListUI);
};

} // namespace khiin::win32

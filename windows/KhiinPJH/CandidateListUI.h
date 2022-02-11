#pragma once

#include "common.h"

namespace khiin::win32 {

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
    virtual void Show() = 0;
    virtual void Hide() = 0;
};

struct CandidateListUIFactory {
    static HRESULT Create(CandidateListUI **ppCandidateListUI);
};

} // namespace khiin::win32

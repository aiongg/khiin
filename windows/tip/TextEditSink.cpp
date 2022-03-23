#include "pch.h"

#include "TextEditSink.h"

namespace khiin::win32::tip {
using namespace winrt;

HRESULT TextEditSink::Initialize(ITfDocumentMgr *docmgr) {
    WINRT_ASSERT(docmgr != nullptr);

    check_hresult(docmgr->GetTop(context.put()));
    m_sink_mgr.Advise(context.get(), this);

    return S_OK;
}

HRESULT TextEditSink::Uninitialize() {
    m_sink_mgr.Unadvise();
    context = nullptr;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfTextEditSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord) {
    return S_OK;
}

} // namespace khiin::win32::tip

#include "pch.h"

#include "TextEditSink.h"

namespace khiin::win32 {

HRESULT TextEditSink::Initialize(ITfDocumentMgr *pDocumentMgr) {
    WINRT_ASSERT(pDocumentMgr != nullptr);

    winrt::check_hresult(pDocumentMgr->GetTop(context.put()));
    textEditSinkMgr.Advise(context.get(), this);

    return S_OK;
}

HRESULT TextEditSink::Uninitialize() {
    textEditSinkMgr.Unadvise();
    context = nullptr;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfTextEditSink
//
//----------------------------------------------------------------------------

STDMETHODIMP TextEditSink::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord) {
    return E_NOTIMPL;
}

} // namespace khiin::win32

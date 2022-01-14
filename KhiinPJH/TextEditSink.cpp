#include "pch.h"

#include "TextEditSink.h"

namespace Khiin {

HRESULT TextEditSink::init(ITfDocumentMgr *pDocumentMgr) {
    WINRT_ASSERT(pDocumentMgr != nullptr);

    auto hr = E_FAIL;

    hr = pDocumentMgr->GetTop(context.put());
    CHECK_RETURN_HRESULT(hr);

    hr = textEditSinkMgr.install(context.get(), this);
    CHECK_RETURN_HRESULT(hr);

    return S_OK;
}

HRESULT TextEditSink::uninit() {
    auto hr = textEditSinkMgr.uninstall();
    CHECK_RETURN_HRESULT(hr);
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

} // namespace Khiin

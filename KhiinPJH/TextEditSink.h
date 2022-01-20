#pragma once

#include "SinkManager.h"

namespace Khiin {

struct TextEditSink : winrt::implements<TextEditSink, ITfTextEditSink> {
    HRESULT Initialize(ITfDocumentMgr *documentMgr);
    HRESULT Uninitialize();

    // Inherited via implements
    virtual STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord) override;

  private:
    SinkManager<ITfTextEditSink> textEditSinkMgr;
    winrt::com_ptr<ITfContext> context;
};

} // namespace Khiin

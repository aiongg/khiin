#pragma once

#include "SinkManager.h"

namespace Khiin {

struct TextEditSink : winrt::implements<TextEditSink, ITfTextEditSink> {
    HRESULT init(ITfDocumentMgr *documentMgr);
    HRESULT uninit();

    // Inherited via implements
    virtual STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord) override;

  private:
    SinkManager<ITfTextEditSink> textEditSinkMgr;
    winrt::com_ptr<ITfContext> context;
};

} // namespace Khiin

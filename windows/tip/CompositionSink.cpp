#include "pch.h"

#include "CompositionSink.h"

namespace khiin::win32::tip {

CompositionSink::CompositionSink(TextService *pService, ITfContext *pContext) {
    service.copy_from(pService);
    context.copy_from(pContext);
}

//+---------------------------------------------------------------------------
//
// ITfCompositionSink
//
//----------------------------------------------------------------------------

STDMETHODIMP CompositionSink::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) {
    TRY_FOR_HRESULT;
    KHIIN_TRACE("");
    service->OnCompositionTerminated(ecWrite, context.get(), pComposition);
    CATCH_FOR_HRESULT;
}

} // namespace khiin::win32

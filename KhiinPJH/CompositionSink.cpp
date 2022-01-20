#include "pch.h"

#include "CompositionSink.h"

namespace Khiin {

CompositionSink::CompositionSink(TextService *pService, ITfContext *pContext) {
    D(__FUNCTIONW__);
    service.copy_from(pService);
    context.copy_from(pContext);
}

//+---------------------------------------------------------------------------
//
// ITfCompositionSink
//
//----------------------------------------------------------------------------

STDMETHODIMP CompositionSink::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) {
    D(__FUNCTIONW__);
    TRY_FOR_HRESULT;
    service->OnCompositionTerminated(ecWrite, context.get(), pComposition);
    CATCH_FOR_HRESULT;
}

} // namespace Khiin

#include "pch.h"

#include "CompositionSink.h"

namespace khiin::win32::tip {

CompositionSink::CompositionSink(TextService *service, ITfContext *context) {
    m_service.copy_from(service);
    m_context.copy_from(context);
}

//+---------------------------------------------------------------------------
//
// ITfCompositionSink
//
//----------------------------------------------------------------------------

STDMETHODIMP CompositionSink::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) {
    TRY_FOR_HRESULT;
    KHIIN_TRACE("");
    m_service->OnCompositionTerminated(ecWrite, m_context.get(), pComposition);
    CATCH_FOR_HRESULT;
}

} // namespace khiin::win32::tip

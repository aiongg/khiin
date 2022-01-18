#pragma once

#include "TextService.h"

namespace Khiin {

struct CompositionSink : winrt::implements<CompositionSink, ITfCompositionSink> {
    CompositionSink(TextService *pService, ITfContext *pContext);
    ~CompositionSink() = default;

    // ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) override;

    DELETE_COPY_AND_ASSIGN(CompositionSink);

  private:
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
};

} // namespace Khiin

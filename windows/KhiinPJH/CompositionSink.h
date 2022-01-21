#pragma once

#include "TextService.h"

namespace khiin::win32 {

struct CompositionSink : winrt::implements<CompositionSink, ITfCompositionSink> {
    CompositionSink(TextService *pService, ITfContext *pContext);
    CompositionSink(const CompositionSink &) = delete;
    CompositionSink &operator=(const CompositionSink &) = delete;
    ~CompositionSink() = default;

    // ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) override;

  private:
    winrt::com_ptr<TextService> service = nullptr;
    winrt::com_ptr<ITfContext> context = nullptr;
};

} // namespace khiin::win32

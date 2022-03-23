#pragma once

#include "TextService.h"

namespace khiin::win32::tip {

struct CompositionSink : winrt::implements<CompositionSink, ITfCompositionSink> {
    CompositionSink(TextService *service, ITfContext *context);
    CompositionSink(const CompositionSink &) = delete;
    CompositionSink &operator=(const CompositionSink &) = delete;
    ~CompositionSink() = default;

    // ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) override;

  private:
    winrt::com_ptr<TextService> m_service = nullptr;
    winrt::com_ptr<ITfContext> m_context = nullptr;
};

} // namespace khiin::win32

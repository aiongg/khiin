#pragma once

#include "pch.h"

namespace Khiin {

template <typename ISink>
class SinkManager {
  public:
    SinkManager() = default;
    SinkManager(const SinkManager &) = delete;
    SinkManager &operator=(const SinkManager &) = delete;
    virtual ~SinkManager() {
        if (installed()) {
            uninstall();
        }
    }

    bool installed() {
        return cookie != TF_INVALID_COOKIE;
    }

    HRESULT install(IUnknown *pHasSource, ISink *sink) {
        WINRT_ASSERT(pHasSource != nullptr);
        WINRT_ASSERT(sink != nullptr);

        auto hr = E_FAIL;
        auto provider = winrt::com_ptr<IUnknown>();
        provider.copy_from(pHasSource);

        auto src = provider.as<ITfSource>();
        hr = src->AdviseSink(__uuidof(ISink), sink, &cookie);

        if (FAILED(hr)) {
            cookie = TF_INVALID_COOKIE;
            CHECK_RETURN_HRESULT(hr);
        }

        source = nullptr;
        source = src;

        return S_OK;
    }

    HRESULT uninstall() {
        if (!installed()) {
            return S_OK;
        }

        auto hr = source->UnadviseSink(cookie);
        CHECK_RETURN_HRESULT(hr);

        cookie = TF_INVALID_COOKIE;
        source = nullptr;

        return S_OK;
    }

  private:
    DWORD cookie = TF_INVALID_COOKIE;
    winrt::com_ptr<ITfSource> source;
};

} // namespace Khiin

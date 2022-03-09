#pragma once

#include "pch.h"

namespace khiin::win32::tip {

template <typename ISink>
class SinkManager {
  public:
    SinkManager() = default;
    SinkManager(const SinkManager &) = delete;
    SinkManager &operator=(const SinkManager &) = delete;
    virtual ~SinkManager() {
        if (advised()) {
            Unadvise();
        }
    }

    void Advise(IUnknown *pHasSource, ISink *sink) {
        WINRT_ASSERT(pHasSource != nullptr);
        WINRT_ASSERT(sink != nullptr);

        auto provider = winrt::com_ptr<IUnknown>();
        provider.copy_from(pHasSource);

        auto src = provider.as<ITfSource>();
        winrt::check_hresult(src->AdviseSink(__uuidof(ISink), sink, &cookie));

        source = nullptr;
        source = src;
    }

    void Unadvise() {
        if (!advised()) {
            return;
        }
        winrt::check_hresult(source->UnadviseSink(cookie));
        cookie = TF_INVALID_COOKIE;
        source = nullptr;
    }

    bool advised() {
        return cookie != TF_INVALID_COOKIE;
    }

  private:
    DWORD cookie = TF_INVALID_COOKIE;
    winrt::com_ptr<ITfSource> source;
};

} // namespace khiin::win32

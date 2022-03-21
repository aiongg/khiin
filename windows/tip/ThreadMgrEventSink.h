#pragma once

namespace khiin::win32::tip {

struct TextService;

struct ThreadMgrEventSink : winrt::implements<ThreadMgrEventSink, IUnknown> {
    ThreadMgrEventSink() = default;
    ThreadMgrEventSink(const ThreadMgrEventSink &) = delete;
    ThreadMgrEventSink &operator=(const ThreadMgrEventSink &) = delete;
    virtual ~ThreadMgrEventSink() = 0;

    static winrt::com_ptr<ThreadMgrEventSink> Create();

    virtual void Initialize(TextService *service) = 0;
    virtual void Uninitialize() = 0;
};

} // namespace khiin::win32

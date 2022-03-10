#pragma once

namespace khiin::win32::tip {

struct TextService;

struct LangBarIndicator : winrt::implements<LangBarIndicator, IUnknown> {
    LangBarIndicator() = default;
    LangBarIndicator(const LangBarIndicator &) = delete;
    LangBarIndicator &operator=(const LangBarIndicator &) = delete;
    ~LangBarIndicator() = default;

    static winrt::com_ptr<LangBarIndicator> Create();
    virtual void Initialize(TextService *pTextService) = 0;
    virtual void Shutdown() = 0;
};

} // namespace khiin::win32

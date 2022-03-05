#pragma once

namespace khiin::win32 {

struct TextService;

struct LangBarIndicator : winrt::implements<LangBarIndicator, IUnknown> {
    LangBarIndicator() = default;
    LangBarIndicator(const LangBarIndicator &) = delete;
    LangBarIndicator &operator=(const LangBarIndicator &) = delete;
    ~LangBarIndicator() = default;

    virtual void Initialize(TextService *pTextService) = 0;
    virtual void Shutdown() = 0;
};

struct LangBarIndicatorFactory {
    static void Create(LangBarIndicator **indicator);
};

} // namespace khiin::win32

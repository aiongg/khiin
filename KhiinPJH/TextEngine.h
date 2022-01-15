#pragma once

namespace Khiin {

struct TextEngine : winrt::implements<TextEngine, IUnknown> {
    TextEngine() = default;
    TextEngine(const TextEngine &) = delete;
    TextEngine &operator=(const TextEngine &) = delete;
    ~TextEngine() = default;

    HRESULT onTestKey(WPARAM wParam, BOOL *pConsumable);
    HRESULT onKey(WPARAM wParam, std::string *output);
    HRESULT clear();

  private:
    std::string buffer{};
};

} // namespace Khiin

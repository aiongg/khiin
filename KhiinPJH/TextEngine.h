#pragma once

namespace Khiin {

struct TextEngine : winrt::implements<TextEngine, IUnknown> {

    HRESULT onTestKey(WPARAM wParam, BOOL *pConsumable);
    HRESULT onKey(WPARAM wParam, std::string *output);
    HRESULT clear();

    HRESULT getCandidates(std::vector<std::string> *pCandidates);

  private:
    std::string buffer{};
    std::vector<std::string> candidates;

    DELETE_COPY_AND_ASSIGN(TextEngine);
};

} // namespace Khiin

#pragma once

namespace Khiin {

struct ITextEngine : winrt::implements<ITextEngine, IUnknown> {
    virtual HRESULT init() = 0;
    virtual HRESULT uninit() = 0;

    virtual HRESULT onTestKey(WPARAM wParam, BOOL *pConsumable) = 0;
    virtual HRESULT onKey(WPARAM wParam, std::string *output) = 0;
    virtual HRESULT clear() = 0;

    virtual HRESULT getCandidates(std::vector<std::string> *pCandidates) = 0;

    DEFAULT_CTOR_DTOR(ITextEngine);
    DELETE_COPY_AND_ASSIGN(ITextEngine);
};

struct TextEngineFactory {
    static HRESULT create(ITextEngine **ppEngine);
};

} // namespace Khiin

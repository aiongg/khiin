#include "pch.h"

#include "ITextEngine.h"

#include "common.h"

namespace Khiin {

struct TextEngineImpl : winrt::implements<TextEngineImpl, ITextEngine> {
    DEFAULT_CTOR_DTOR(TextEngineImpl);

    virtual HRESULT init() override {
        return S_OK;
    }

    virtual HRESULT uninit() override {
        return S_OK;
    }

    virtual HRESULT onTestKey(WPARAM wParam, BOOL *pConsumable) {
        if (wParam == 0x41) {
            *pConsumable = true;
        } else {
            *pConsumable = false;
        }
        return S_OK;
    }

    virtual HRESULT onKey(WPARAM wParam, std::string *pOutput) {
        if (wParam == 0x41) {
            buffer += 'r';
        }

        *pOutput = buffer;
        return S_OK;
    }

    virtual HRESULT clear() {
        buffer.clear();
        return S_OK;
    }

    virtual HRESULT getCandidates(std::vector<std::string> *pCandidates) {
        if (candidates.empty()) {
            candidates.push_back(u8"枝");
            candidates.push_back(u8"乩");
            candidates.push_back(u8"機");
            candidates.push_back(u8"机");
            candidates.push_back(u8"箕");
            candidates.push_back(u8"飢");
            candidates.push_back(u8"幾");
            candidates.push_back(u8"支");
            candidates.push_back(u8"居");
            candidates.push_back(u8"基");
            candidates.push_back(u8"奇");
            candidates.push_back(u8"姬");
            candidates.push_back(u8"畿");
            candidates.push_back(u8"裾");
        }

        *pCandidates = candidates;
        return S_OK;
    }

  private:
    std::string buffer{};
    std::vector<std::string> candidates;
};

HRESULT TextEngineFactory::create(ITextEngine **ppEngine) {
    as_self<ITextEngine>(winrt::make_self<TextEngineImpl>()).copy_to(ppEngine);
    return S_OK;
}

} // namespace Khiin

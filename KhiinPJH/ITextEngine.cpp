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

    virtual HRESULT onTestKey(KeyEvent keyEvent, BOOL *pConsumable) {
        if (keyEvent.ascii() == 'a') {
            *pConsumable = true;
        } else {
            *pConsumable = false;
        }
        return S_OK;
    }

    virtual HRESULT onKey(KeyEvent keyEvent) {
        if (keyEvent.ascii() == 'a') {
            buffer_ += 'r';
        }

        return S_OK;
    }

    virtual HRESULT clear() {
        buffer_.clear();
        return S_OK;
    }

    virtual std::string buffer() {
        return buffer_;
    }

    virtual HRESULT candidates(std::vector<std::string> *pCandidates) {
        if (candidates_.empty()) {
            candidates_.push_back(u8"枝");
            candidates_.push_back(u8"乩");
            candidates_.push_back(u8"機");
            candidates_.push_back(u8"机");
            candidates_.push_back(u8"箕");
            candidates_.push_back(u8"飢");
            candidates_.push_back(u8"幾");
            candidates_.push_back(u8"支");
            candidates_.push_back(u8"居");
            candidates_.push_back(u8"基");
            candidates_.push_back(u8"奇");
            candidates_.push_back(u8"姬");
            candidates_.push_back(u8"畿");
            candidates_.push_back(u8"裾");
        }

        *pCandidates = candidates_;
        return S_OK;
    }

  private:
    std::string buffer_{};
    std::vector<std::string> candidates_;
};

HRESULT TextEngineFactory::create(ITextEngine **ppEngine) {
    as_self<ITextEngine>(winrt::make_self<TextEngineImpl>()).copy_to(ppEngine);
    return S_OK;
}

} // namespace Khiin

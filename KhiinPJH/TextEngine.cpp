#include "pch.h"

#include "TextEngine.h"

#include "common.h"

namespace Khiin {

struct TextEngineImpl : winrt::implements<TextEngineImpl, TextEngine> {
    TextEngineImpl() = default;
    TextEngineImpl(const TextEngineImpl &) = delete;
    TextEngineImpl &operator=(const TextEngineImpl &) = delete;
    ~TextEngineImpl() = default;

    virtual void Initialize() override {}

    virtual void Uninitialize() override {}

    virtual void TestKey(KeyEvent keyEvent, BOOL *pConsumable) {
        if (keyEvent.ascii() == 'a') {
            *pConsumable = true;
        } else {
            *pConsumable = false;
        }
    }

    virtual void OnKey(KeyEvent keyEvent) {
        if (keyEvent.ascii() == 'a') {
            buffer_ += 'r';
        }
    }

    virtual void Reset() {
        buffer_.clear();
    }

    virtual std::string buffer() {
        return buffer_;
    }

    virtual std::vector<std::string> &candidates() {
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

        return candidates_;
    }

  private:
    std::string buffer_{};
    std::vector<std::string> candidates_;
};

HRESULT TextEngineFactory::Create(TextEngine **ppEngine) {
    as_self<TextEngine>(winrt::make_self<TextEngineImpl>()).copy_to(ppEngine);
    return S_OK;
}

} // namespace Khiin

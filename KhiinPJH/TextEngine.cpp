#include "pch.h"

//#include <taikey/engine.h>

#include "TextEngine.h"

#include "common.h"

namespace khiin::win32 {

struct TextEngineImpl : winrt::implements<TextEngineImpl, TextEngine> {
    TextEngineImpl() = default;
    TextEngineImpl(const TextEngineImpl &) = delete;
    TextEngineImpl &operator=(const TextEngineImpl &) = delete;
    ~TextEngineImpl() = default;

    virtual void Initialize() override {
        //engine_ = std::make_unique<taikey::Engine>();
    }

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
            candidates_.push_back(u8"bîn-ná-chài");
            candidates_.push_back(u8"明旦再");
            candidates_.push_back(u8"明那再");
            candidates_.push_back(u8"明仔再");
            candidates_.push_back(u8"bîn-ná");
            candidates_.push_back(u8"明旦");
            candidates_.push_back(u8"明那");
            candidates_.push_back(u8"明仔");
            candidates_.push_back(u8"bīn");
            candidates_.push_back(u8"面");
            candidates_.push_back(u8"bín");
            candidates_.push_back(u8"敏");
            candidates_.push_back(u8"抿");
            candidates_.push_back(u8"bîn");
            candidates_.push_back(u8"緡");
            candidates_.push_back(u8"珉");
            candidates_.push_back(u8"黽");
            candidates_.push_back(u8"敏");
            candidates_.push_back(u8"愍");
            candidates_.push_back(u8"憫");
            candidates_.push_back(u8"閔");
            candidates_.push_back(u8"眠");
        }

        return candidates_;
    }

  private:
    std::string buffer_{};
    std::vector<std::string> candidates_;
    //std::unique_ptr<taikey::Engine> engine_ = nullptr;
};

HRESULT TextEngineFactory::Create(TextEngine **ppEngine) {
    as_self<TextEngine>(winrt::make_self<TextEngineImpl>()).copy_to(ppEngine);
    return S_OK;
}

} // namespace khiin::win32

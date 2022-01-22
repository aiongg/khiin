#include "pch.h"

#include "TextEngine.h"

#include <filesystem>

#include <engine/engine.h>
#include <utf8cpp/utf8/cpp17.h>

#include "common.h"
#include "DllModule.h"

namespace {
volatile HMODULE g_module = nullptr;
}

namespace khiin::win32 {

namespace fs = std::filesystem;

engine::KeyCode TranslateCode(win32::KeyEvent ke) {
    if (ke.ascii()) {
        return (static_cast<engine::KeyCode>(ke.ascii()));
    }

    switch (ke.keyCode()) {
    case VK_RETURN:
        return engine::KeyCode::ENTER;
    case VK_RIGHT:
        return engine::KeyCode::RIGHT;
    }

    return engine::KeyCode::ENTER;
}

fs::path DefaultResourceDirectory() {
    WINRT_ASSERT(g_module);
    auto dll_path = std::wstring(MAX_PATH, '?');
    auto len = ::GetModuleFileName(g_module, &dll_path[0], MAX_PATH);
    if (len == 0) {
        throw winrt::hresult_error(::GetLastError());
    }
    auto tmp = std::u16string(dll_path.cbegin(), dll_path.cbegin() + len);
    auto path = fs::path(utf8::utf16to8(tmp));
    path.replace_filename("resources");
    return path;
}

struct TextEngineImpl : winrt::implements<TextEngineImpl, TextEngine> {
    TextEngineImpl() = default;
    TextEngineImpl(const TextEngineImpl &) = delete;
    TextEngineImpl &operator=(const TextEngineImpl &) = delete;
    ~TextEngineImpl() = default;

    virtual void Initialize() override {
        engine_ = std::make_unique<engine::Engine>(DefaultResourceDirectory().string());
    }

    virtual void Uninitialize() override {}

    virtual void TestKey(KeyEvent keyEvent, BOOL *pConsumable) {
        *pConsumable = engine_->consumable(TranslateCode(keyEvent));
    }

    virtual void OnKey(KeyEvent keyEvent) {
        //buffer_ += 'r';
        auto rv = engine_->onKeyDown(TranslateCode(keyEvent), display_data);
        if (rv == RetVal::NotConsumed || rv == RetVal::Cancelled) {
            // do something
        }
    }

    virtual void Reset() {
        engine_->Reset();
    }

    virtual std::string buffer() {
        return display_data.buffer;
    }

    virtual std::vector<std::string> &candidates() {
        candidates_.clear();
        for (auto &c : display_data.candidates) {
            candidates_.push_back(c.text);
        }
        return candidates_;
    }

  private:
    std::string buffer_{};
    std::vector<std::string> candidates_;
    std::unique_ptr<engine::Engine> engine_ = nullptr;
    engine::ImeDisplayData display_data{};
};

//+---------------------------------------------------------------------------
//
// TextEngineFactory
//
//----------------------------------------------------------------------------

void TextEngineFactory::OnDllProcessAttach(HMODULE module) {
    DllModule::AddRef();
    g_module = module;
}

void TextEngineFactory::OnDllProcessDetach(HMODULE module) {
    g_module = nullptr;
    DllModule::Release();
}

HRESULT TextEngineFactory::Create(TextEngine **ppEngine) {
    as_self<TextEngine>(winrt::make_self<TextEngineImpl>()).copy_to(ppEngine);
    return S_OK;
}

} // namespace khiin::win32

#include "pch.h"

#include "TextEngine.h"

#include <filesystem>
#include <unordered_map>

#include <engine/engine.h>

#include "DllModule.h"
#include "Utils.h"
#include "common.h"

namespace {

using namespace khiin::engine;

volatile HMODULE g_module = nullptr;

static std::unordered_map<int, KeyCode> kWindowsToKhiinKeyCode = {
    // clang-format off
    {VK_BACK, KeyCode::BACK},
    {VK_TAB, KeyCode::TAB},
    {VK_RETURN, KeyCode::ENTER},
    {VK_ESCAPE, KeyCode::ESC},
    {VK_SPACE, KeyCode::SPACE},
    {VK_END, KeyCode::END},
    {VK_HOME, KeyCode::HOME},
    {VK_LEFT, KeyCode::LEFT},
    {VK_UP, KeyCode::UP},
    {VK_RIGHT, KeyCode::RIGHT},
    {VK_DOWN, KeyCode::DOWN},
    {VK_DELETE, KeyCode::DEL}
    // clang-format on
};

} // namespace

namespace khiin::win32 {

namespace fs = std::filesystem;

engine::KeyCode TranslateCode(win32::KeyEvent ke) {
    if (ke.ascii()) {
        return (static_cast<engine::KeyCode>(ke.ascii()));
    }

    if (auto idx = kWindowsToKhiinKeyCode.find(ke.keyCode()); idx != kWindowsToKhiinKeyCode.end()) {
        return idx->second;
    }

    return engine::KeyCode::UNKNOWN;
}

fs::path DefaultResourceDirectory() {
    WINRT_ASSERT(g_module);
    auto dll_path = std::wstring(MAX_PATH, '?');
    auto len = ::GetModuleFileName(g_module, &dll_path[0], MAX_PATH);
    if (len == 0) {
        throw winrt::hresult_error(::GetLastError());
    }
    auto path = fs::path(Utils::Narrow(dll_path));
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

    virtual Action TestKey(KeyEvent keyEvent) {
        Action a{};
        auto consumable = engine_->TestConsumable(TranslateCode(keyEvent));
        if (!consumable) {
            a.consumed = false;
            a.compose_message = Message::Commit;
            a.candidate_message = Message::HideCandidates;
            Reset();
        } else {
            a.consumed = true;
            a.compose_message = Message::Noop;
            a.candidate_message = Message::Noop;
        }
        return a;
    }

    virtual Action OnKey(KeyEvent keyEvent) {
        auto rv = engine_->onKeyDown(TranslateCode(keyEvent), display_data);
        buffer_ = display_data.buffer;
        candidates_.clear();
        if (display_data.candidates.size()) {
            for (auto &c : display_data.candidates) {
                candidates_.push_back(c.text);
            }
        }

        Action a{};

        switch (rv) {
        case RetVal::Cancelled:
            a.compose_message = Message::CancelComposition;
            a.candidate_message = Message::HideCandidates;
            break;
        case RetVal::Consumed:
            a.compose_message = Message::Compose;
            a.buffer_text = buffer_;

            if (candidates_.size()) {
                a.candidate_message = Message::ShowCandidates;
            } else {
                a.candidate_message = Message::HideCandidates;
            }

            a.candidate_list = &candidates_;
            break;
        default:
            a.compose_message = Message::Commit;
            a.buffer_text = buffer_;
            a.candidate_message = Message::HideCandidates;
        }

        return a;
    }

    virtual void Reset() {
        engine_->Reset();
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

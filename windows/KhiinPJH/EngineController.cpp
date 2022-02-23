#include "pch.h"

#include "EngineController.h"

#include <filesystem>
#include <unordered_map>

#include "DllModule.h"
#include "EditSession.h"
#include "Utils.h"
#include "common.h"
#include "engine/engine.h"

namespace {

using namespace khiin::engine;
using namespace khiin::messages;

volatile HMODULE g_module = nullptr;

std::string const kDbFilename = "khiin_test.db";

static std::unordered_map<int, SpecialKey> kWindowsToKhiinKeyCode = {
    // clang-format off
    {VK_BACK, SpecialKey::SK_BACKSPACE},
    {VK_TAB, SpecialKey::SK_TAB},
    {VK_RETURN, SpecialKey::SK_ENTER},
    {VK_ESCAPE, SpecialKey::SK_ESC},
    {VK_SPACE, SpecialKey::SK_SPACE},
    {VK_END, SpecialKey::SK_END},
    {VK_HOME, SpecialKey::SK_HOME},
    {VK_LEFT, SpecialKey::SK_LEFT},
    {VK_UP, SpecialKey::SK_UP},
    {VK_RIGHT, SpecialKey::SK_RIGHT},
    {VK_DOWN, SpecialKey::SK_DOWN},
    {VK_DELETE, SpecialKey::SK_DEL}
    // clang-format on
};

} // namespace

namespace khiin::win32 {

using namespace engine;
using namespace messages;
namespace fs = std::filesystem;

void TranslateKeyEvent(win32::KeyEvent *e1, messages::KeyEvent *e2) {
    if (e1->ascii()) {
        e2->set_key_code(e1->ascii());
        e2->set_special_key(SpecialKey::SK_NONE);
    }

    if (auto idx = kWindowsToKhiinKeyCode.find(e1->keyCode()); idx != kWindowsToKhiinKeyCode.end()) {
        e2->set_special_key(idx->second);
    }
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
    path /= kDbFilename;
    return path;
}

struct EngineControllerImpl : winrt::implements<EngineControllerImpl, EngineController> {
    EngineControllerImpl() = default;
    EngineControllerImpl(const EngineControllerImpl &) = delete;
    EngineControllerImpl &operator=(const EngineControllerImpl &) = delete;
    ~EngineControllerImpl() = default;

    virtual void Initialize() override {
        m_engine = std::unique_ptr<Engine>(Engine::Create(DefaultResourceDirectory().string()));
    }

    virtual void Uninitialize() override {
        m_engine.reset(nullptr);
    }

    virtual Command TestKey(KeyEvent win_key_event) override {
        auto cmd = Command();
        cmd.set_type(CommandType::TEST_SEND_KEY);
        auto key_event = cmd.mutable_input()->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        m_engine->SendCommand(&cmd);

        if (!cmd.output().consumable()) {
            Reset();
        }

        return cmd;
    }

    virtual Command OnKey(KeyEvent win_key_event) override {
        auto cmd = Command();
        cmd.set_type(CommandType::SEND_KEY);
        auto key_event = cmd.mutable_input()->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        m_engine->SendCommand(&cmd);
        return cmd;
    }

    virtual Command SelectCandidate(int32_t candidate_id) override {
        auto cmd = Command();
        cmd.set_type(CommandType::SELECT_CANDIDATE);
        cmd.mutable_input()->set_candidate_id(candidate_id);
        m_engine->SendCommand(&cmd);
        return cmd;
    }

    virtual Command FocusCandidate(int32_t candidate_id) override {
        auto cmd = Command();
        cmd.set_type(CommandType::FOCUS_CANDIDATE);
        cmd.mutable_input()->set_candidate_id(candidate_id);
        m_engine->SendCommand(&cmd);
        return cmd;
    }

    virtual void Reset() {
        auto cmd = Command();
        cmd.set_type(CommandType::RESET);
        m_engine->SendCommand(&cmd);
    }

  private:
    std::string buffer_{};
    std::vector<std::string> candidates_;
    std::unique_ptr<Engine> m_engine = nullptr;
};

//+---------------------------------------------------------------------------
//
// TextEngineFactory
//
//----------------------------------------------------------------------------

void EngineControllerFactory::OnDllProcessAttach(HMODULE module) {
    DllModule::AddRef();
    g_module = module;
}

void EngineControllerFactory::OnDllProcessDetach(HMODULE module) {
    g_module = nullptr;
    DllModule::Release();
}

void EngineControllerFactory::Create(EngineController **ppEngineCtrl) {
    as_self<EngineController>(winrt::make_self<EngineControllerImpl>()).copy_to(ppEngineCtrl);
}

} // namespace khiin::win32

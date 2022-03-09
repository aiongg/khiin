#include "pch.h"

#include "EngineController.h"

#include <filesystem>
#include <unordered_map>

#include <engine/Engine.h>

#include "DllModule.h"
#include "EditSession.h"
#include "Files.h"
#include "Utils.h"
#include "common.h"
#include "proto/proto.h"

namespace khiin::win32 {

namespace {
using namespace khiin::engine;
using namespace khiin::proto;

volatile HMODULE g_module = nullptr;

constexpr std::string_view kDbFilename = "khiin.db";

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

using namespace engine;
using namespace proto;
namespace fs = std::filesystem;

void TranslateKeyEvent(win32::KeyEvent *e1, proto::KeyEvent *e2) {
    if (e1->ascii()) {
        e2->set_key_code(e1->ascii());
        e2->set_special_key(SpecialKey::SK_NONE);
    }

    if (auto idx = kWindowsToKhiinKeyCode.find(e1->keyCode()); idx != kWindowsToKhiinKeyCode.end()) {
        e2->set_special_key(idx->second);
    }
}

struct EngineControllerImpl : winrt::implements<EngineControllerImpl, EngineController> {
    EngineControllerImpl() = default;
    EngineControllerImpl(const EngineControllerImpl &) = delete;
    EngineControllerImpl &operator=(const EngineControllerImpl &) = delete;
    ~EngineControllerImpl() = default;

    virtual void Initialize() override {
        auto dbfile = Files::GetFilePath(g_module, kDbFilename);
        m_engine = std::unique_ptr<Engine>(Engine::Create(dbfile.string()));
    }

    virtual void Uninitialize() override {
        m_engine.reset(nullptr);
    }

    virtual Command *TestKey(KeyEvent win_key_event) override {
        auto cmd = NewCommand();
        auto request = cmd->mutable_request();
        request->set_type(CommandType::TEST_SEND_KEY);
        auto key_event = request->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        m_engine->SendCommand(cmd);

        if (!cmd->response().consumable()) {
            Reset();
        }

        return cmd;
    }

    virtual Command *OnKey(KeyEvent win_key_event) override {
        auto cmd = NewCommand();
        auto request = cmd->mutable_request();
        request->set_type(CommandType::SEND_KEY);
        auto key_event = request->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        m_engine->SendCommand(cmd);
        return cmd;
    }

    virtual Command *SelectCandidate(int32_t candidate_id) override {
        auto cmd = NewCommand();
        auto request = cmd->mutable_request();
        request->set_type(CommandType::SELECT_CANDIDATE);
        request->set_candidate_id(candidate_id);
        m_engine->SendCommand(cmd);
        return cmd;
    }

    virtual Command *FocusCandidate(int32_t candidate_id) override {
        auto cmd = NewCommand();
        auto request = cmd->mutable_request();
        request->set_type(CommandType::FOCUS_CANDIDATE);
        request->set_candidate_id(candidate_id);
        m_engine->SendCommand(cmd);
        return cmd;
    }

    virtual void Reset() {
        auto cmd = NewCommand();
        auto request = cmd->mutable_request();
        request->set_type(CommandType::RESET);
        m_engine->SendCommand(cmd);
    }

  private:
    Command *NewCommand() {
        auto cmd = Command::default_instance().New();
        m_prev_command = std::unique_ptr<Command>(cmd);
        return m_prev_command.get();
    }

    std::string buffer_{};
    std::vector<std::string> candidates_;
    std::unique_ptr<Engine> m_engine = nullptr;
    std::unique_ptr<Command> m_prev_command = nullptr;
};

//+---------------------------------------------------------------------------
//
// TextEngineFactory
//
//----------------------------------------------------------------------------

void EngineControllerFactory::OnDllProcessAttach(HMODULE module) {
    tip::DllModule::AddRef();
    g_module = module;
}

void EngineControllerFactory::OnDllProcessDetach(HMODULE module) {
    g_module = nullptr;
    tip::DllModule::Release();
}

void EngineControllerFactory::Create(EngineController **ppEngineCtrl) {
    as_self<EngineController>(winrt::make_self<EngineControllerImpl>()).copy_to(ppEngineCtrl);
}

} // namespace khiin::win32

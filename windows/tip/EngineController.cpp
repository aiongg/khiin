#include "pch.h"

#include "EngineController.h"

#include <filesystem>
#include <unordered_map>

#include <engine/Engine.h>

#include "Config.h"
#include "DllModule.h"
#include "EditSession.h"
#include "Files.h"
#include "TextService.h"
#include "Utils.h"
#include "common.h"
#include "proto/proto.h"

namespace khiin::win32::tip {
namespace {
namespace fs = std::filesystem;
using namespace winrt;
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

void TranslateKeyEvent(win32::KeyEvent *e1, proto::KeyEvent *e2) {
    if (e1->ascii()) {
        e2->set_key_code(e1->ascii());
        e2->set_special_key(SpecialKey::SK_NONE);
    }

    if (auto idx = kWindowsToKhiinKeyCode.find(e1->keyCode()); idx != kWindowsToKhiinKeyCode.end()) {
        e2->set_special_key(idx->second);
    }
}

struct EngineControllerImpl : winrt::implements<EngineControllerImpl, EngineController>, ConfigChangeListener {
    EngineControllerImpl() = default;
    EngineControllerImpl(const EngineControllerImpl &) = delete;
    EngineControllerImpl &operator=(const EngineControllerImpl &) = delete;
    ~EngineControllerImpl() = default;

    virtual void Initialize(TextService *pService) override {
        m_service.copy_from(pService);
        auto dbfile = Files::GetFilePath(g_module, kDbFilename);
        m_engine = std::unique_ptr<Engine>(Engine::Create(dbfile.string()));
        m_service->RegisterConfigChangeListener(this);
    }

    virtual void Uninitialize() override {
        m_engine.reset(nullptr);
        m_service = nullptr;
    }

    virtual Command *TestKey(KeyEvent win_key_event) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_TEST_SEND_KEY);
        auto key_event = request->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        m_engine->SendCommand(cmd);

        if (!cmd->response().consumable()) {
            Reset();
        }

        return SendCommand(cmd);
    }

    virtual Command *OnKey(KeyEvent win_key_event) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_SEND_KEY);
        auto key_event = request->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        return SendCommand(cmd);
    }

    virtual Command *SelectCandidate(int32_t candidate_id) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_SELECT_CANDIDATE);
        request->set_candidate_id(candidate_id);
        return SendCommand(cmd);
    }

    virtual Command *FocusCandidate(int32_t candidate_id) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_FOCUS_CANDIDATE);
        request->set_candidate_id(candidate_id);
        return SendCommand(cmd);
    }

    virtual void Reset() {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_RESET);
        SendCommand(cmd);
    }

    //+---------------------------------------------------------------------------
    //
    // ConfigChangeListener
    //
    //----------------------------------------------------------------------------

    virtual void OnConfigChanged(proto::AppConfig *config) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_SET_CONFIG);
        request->mutable_config()->CopyFrom(*config);
        SendCommand(cmd);
    }

  private:
    Command *SendCommand(Command *cmd) {
        m_engine->SendCommand(cmd);
        m_prev_command = std::unique_ptr<Command>(cmd);
        return m_prev_command.get();
    }

    std::string buffer_{};
    std::vector<std::string> candidates_;
    std::unique_ptr<Engine> m_engine = nullptr;
    std::unique_ptr<Command> m_prev_command = nullptr;
    com_ptr<TextService> m_service = nullptr;
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

} // namespace khiin::win32::tip

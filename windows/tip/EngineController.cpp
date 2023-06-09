#include "pch.h"

#include "EngineController.h"

#include <filesystem>
#include <unordered_map>

#include "engine/Engine.h"
#include "proto/proto.h"

#include "Config.h"
#include "DllModule.h"
#include "EditSession.h"
#include "TextService.h"
#include "Utils.h"
#include "common.h"

namespace khiin::win32::tip {
namespace {
namespace fs = std::filesystem;
using namespace winrt;
using namespace khiin::engine;
using namespace khiin::proto;

volatile HMODULE g_module = nullptr;

#ifdef _DEBUG
const std::wstring kDbFilename = L"khiin_test.db";
#else
const std::wstring kDbFilename = L"khiin.db";
#endif

const std::unordered_map<int, SpecialKey> kWindowsToKhiinKeyCode = {
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

std::string GetDatabaseFile() {
    return Utils::Narrow(Config::GetKnownFile(KhiinFile::Database, g_module, kDbFilename));
}

} // namespace

void TranslateKeyEvent(win32::KeyEvent *e1, proto::KeyEvent *e2) {
    if (e1->ascii() > 0) {
        e2->set_key_code(e1->ascii());
        e2->set_special_key(SpecialKey::SK_NONE);
    }

    if (auto idx = kWindowsToKhiinKeyCode.find(e1->keycode()); idx != kWindowsToKhiinKeyCode.end()) {
        e2->set_special_key(idx->second);
    }

    if (e1->key_down(VK_SHIFT)) {
        e2->add_modifier_keys(ModifierKey::MODK_SHIFT);
    }

    if (e1->key_down(VK_CONTROL)) {
        e2->add_modifier_keys(ModifierKey::MODK_CTRL);
    }

    if (e1->key_down(VK_MENU)) {
        e2->add_modifier_keys(ModifierKey::MODK_ALT);
    }
}

struct EngineControllerImpl : winrt::implements<EngineControllerImpl, EngineController>, ConfigChangeListener {
    EngineControllerImpl() = default;
    EngineControllerImpl(const EngineControllerImpl &) = delete;
    EngineControllerImpl &operator=(const EngineControllerImpl &) = delete;
    ~EngineControllerImpl() = default;

    void Initialize(TextService *service) override {
        m_service.copy_from(service);
        m_service->RegisterConfigChangeListener(this);
        m_engine = Engine::Create(GetDatabaseFile());
    }

    void Uninitialize() override {
        m_service = nullptr;
    }

    Command *TestKey(KeyEvent win_key_event) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_TEST_SEND_KEY);
        auto key_event = request->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        m_engine->SendCommand(cmd);

        KHIIN_DEBUG("error code: {}", cmd->response().error() == ErrorCode::FAIL);

        if (!cmd->response().consumable()) {
            Reset();
        }

        return cmd;
    }

    Command *OnKey(KeyEvent win_key_event) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_SEND_KEY);
        auto key_event = request->mutable_key_event();
        TranslateKeyEvent(&win_key_event, key_event);
        return SendCommand(cmd);
    }

    Command *SelectCandidate(int32_t candidate_id) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_SELECT_CANDIDATE);
        request->set_candidate_id(candidate_id);
        return SendCommand(cmd);
    }

    Command *FocusCandidate(int32_t candidate_id) override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_FOCUS_CANDIDATE);
        request->set_candidate_id(candidate_id);
        return SendCommand(cmd);
    }

    Command *Commit() override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_COMMIT);
        return SendCommand(cmd);
    }

    void ResetUserData() override {
        auto cmd = new Command();
        cmd->mutable_request()->set_type(CMD_RESET_USER_DATA);
        SendCommand(cmd);
    }

    void Reset() override {
        auto cmd = new Command();
        auto request = cmd->mutable_request();
        request->set_type(CMD_RESET);
        SendCommand(cmd);
    }

    CandidateList *LoadEmojis() override {
        if (m_emojis.candidates_size() == 0) {
            auto cmd = new Command();
            cmd->mutable_request()->set_type(CMD_LIST_EMOJIS);
            SendCommand(cmd);
            m_emojis.CopyFrom(cmd->response().candidate_list());
        }
        return &m_emojis;
    }

    //+---------------------------------------------------------------------------
    //
    // ConfigChangeListener
    //
    //----------------------------------------------------------------------------

    void OnConfigChanged(proto::AppConfig *config) override {
        if (auto wfile = Config::GetKnownFile(KhiinFile::UserDb, g_module); !wfile.empty() && fs::exists(wfile)) {
            auto file = Utils::Narrow(wfile);
            m_engine->LoadUserDictionary(file);
        }

        auto *cmd = new Command();
        auto *request = cmd->mutable_request();
        request->set_type(CMD_SET_CONFIG);
        request->mutable_config()->CopyFrom(*config);
        SendCommand(cmd);
    }

  private:
    Command *SendCommand(Command *cmd) {
        if (cmd->request().key_event().special_key() == SK_BACKSPACE) {
            auto x = 3;
        }
        m_engine->SendCommand(cmd);
        m_prev_command = std::unique_ptr<Command>(cmd);
        return m_prev_command.get();
    }

    std::unique_ptr<Engine> m_engine = nullptr;
    com_ptr<TextService> m_service = nullptr;
    std::unique_ptr<Command> m_prev_command = nullptr;
    CandidateList m_emojis;
};

void EngineController::OnDllProcessAttach(HMODULE module) {
    tip::DllModule::AddRef();
    g_module = module;
}

void EngineController::OnDllProcessDetach(HMODULE module) {
    g_module = nullptr;
    tip::DllModule::Release();
}

com_ptr<EngineController> EngineController::Create() {
    return as_self<EngineController>(winrt::make_self<EngineControllerImpl>());
}

} // namespace khiin::win32::tip

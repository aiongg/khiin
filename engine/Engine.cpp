//#ifdef _WIN32
//#ifdef _MSC_VER
//#pragma execution_character_set("utf-8")
//#endif
//#endif

#include "Engine.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "proto/proto.h"
#include "utf8cpp/utf8.h"

#include "config/Config.h"
#include "config/KeyConfig.h"
#include "data/Database.h"
#include "data/Dictionary.h"
#include "data/UserDictionary.h"
#include "input/BufferMgr.h"
#include "input/SyllableParser.h"
#include "utils/logger.h"
#include "utils/utils.h"

namespace khiin::engine {
namespace {

using namespace khiin::proto;
namespace fs = std::filesystem;

bool ModKeyPressed(KeyEvent key_event, ModifierKey modifier) {
    auto const &mods = key_event.modifier_keys();
    return std::any_of(mods.cbegin(), mods.cend(), [&](auto m) {
        return m == modifier;
    });
}

class EngineImpl final : public Engine {
  public:
    EngineImpl() = default;
    EngineImpl(std::string dbfile) : m_dbfilename(dbfile) {}
    EngineImpl(EngineImpl const &rhs) = delete;
    EngineImpl &operator=(EngineImpl const &rhs) = delete;
    ~EngineImpl() = default;

    void Initialize() {
        logger::info("Test logging Initialize {}", "x");
        std::lock_guard<std::mutex> lock(m_mutex);

        Reinit();
    }

    void SendCommand(Command *command) override {
        SendCommand(command->mutable_request(), command->mutable_response());
    }

    void SendCommand(const Request *request, Response *response) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        decltype(&EngineImpl::HandleNone) handler;

        if (auto it = m_cmd_handlers.find(request->type()); it != m_cmd_handlers.end()) {
            handler = it->second;
        } else {
            handler = &EngineImpl::HandleNone;
        }

        (this->*handler)(request, response);
    }

    void LoadDictionary(std::string const &file_path) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (auto curr_db = m_database->CurrentConnection(); curr_db != file_path) {
            m_dbfilename = file_path;
            m_config_change_listeners.clear();
            Reinit();
        }
    }

    void LoadUserDictionary(std::string file_path) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (file_path.empty()) {
            m_userdict = nullptr;
        } else {
            if (fs::exists(fs::path(file_path))) {
                m_userdict = UserDictionary::Create(file_path);
            }
        }
    }

    BufferMgr *buffer_mgr() override {
        return m_buffer_mgr.get();
    }

    Database *database() override {
        return m_database.get();
    }

    KeyConfig *keyconfig() override {
        return m_keyconfig.get();
    }

    SyllableParser *syllable_parser() override {
        return m_syllable_parser.get();
    }

    Dictionary *dictionary() override {
        return m_dictionary.get();
    }

    UserDictionary *user_dict() override {
        return m_userdict.get();
    }

    Config *config() override {
        return m_config.get();
    }

    void RegisterConfigChangedListener(ConfigChangeListener *listener) override {
        m_config_change_listeners.push_back(listener);
    }

  private:
    void Reinit() {
        auto db_path = fs::path(m_dbfilename);
        if (fs::exists(db_path)) {
            m_database = Database::Connect(db_path.string());
        } else {
            m_database = Database::TestDb();
        }

        m_config = Config::Default();
        m_keyconfig = KeyConfig::Create(this);
        m_syllable_parser = SyllableParser::Create(this);
        m_dictionary = Dictionary::Create(this);
        m_buffer_mgr = BufferMgr::Create(this);

        if (m_cmd_handlers.empty()) {
            m_cmd_handlers[CMD_RESET] = &EngineImpl::HandleReset;
            m_cmd_handlers[CMD_COMMIT] = &EngineImpl::HandleCommit;
            m_cmd_handlers[CMD_TEST_SEND_KEY] = &EngineImpl::HandleTestSendKey;
            m_cmd_handlers[CMD_SEND_KEY] = &EngineImpl::HandleSendKey;
            m_cmd_handlers[CMD_SELECT_CANDIDATE] = &EngineImpl::HandleSelectCandidate;
            m_cmd_handlers[CMD_FOCUS_CANDIDATE] = &EngineImpl::HandleFocusCandidate;
            m_cmd_handlers[CMD_SET_CONFIG] = &EngineImpl::HandleSetConfig;
            m_cmd_handlers[CMD_LIST_EMOJIS] = &EngineImpl::HandleListEmojis;
            m_cmd_handlers[CMD_RESET_USER_DATA] = &EngineImpl::HandleResetUserData;
        }

        NotifyConfigChangeListeners();
        m_dictionary->Initialize();
    }

    //+---------------------------------------------------------------------------
    //
    // Command-type handlers
    //
    //----------------------------------------------------------------------------

    void HandleNone(const Request *request, Response *response) {}

    void HandleReset(const Request *request, Response *response) {
        spdlog::debug("HandleReset");
        m_buffer_mgr->Clear();
    }

    void HandleSendKey(const Request *request, Response *response) {
        spdlog::debug("HandleSendKey");
        auto const &key = request->key_event();

        switch (key.special_key()) {
        case SK_NONE: {
            auto key_code = request->key_event().key_code();
            if (isprint(key_code) != 0) {
                m_buffer_mgr->Insert(static_cast<char>(key_code));
            }
            break;
        }
        case SK_RIGHT: {
            m_buffer_mgr->HandleLeftRight(CursorDirection::R);
            break;
        }
        case SK_LEFT: {
            m_buffer_mgr->HandleLeftRight(CursorDirection::L);
            break;
        }
        case SK_TAB:
            if (ModKeyPressed(key, MODK_SHIFT)) {
                m_buffer_mgr->FocusPrevCandidate();
            } else {
                m_buffer_mgr->FocusNextCandidate();
            }
            break;
        case SK_DOWN: {
            m_buffer_mgr->FocusNextCandidate();
            break;
        }
        case SK_UP: {
            m_buffer_mgr->FocusPrevCandidate();
            break;
        }
        case SK_ENTER: {
            if (m_buffer_mgr->HandleSelectOrCommit()) {
                HandleCommit(request, response);
                return;
            }
            break;
        }
        case SK_BACKSPACE: {
            if (m_buffer_mgr->IsEmpty()) {
                response->set_consumable(false);
            } else {
                m_buffer_mgr->Erase(CursorDirection::L);
            }
            break;
        }
        case SK_DEL: {
            if (m_buffer_mgr->IsEmpty()) {
                response->set_consumable(false);
            } else {
                m_buffer_mgr->Erase(CursorDirection::R);
            }
            break;
        }
        case SK_SPACE: {
            if (m_buffer_mgr->IsEmpty()) {
                response->set_consumable(false);
            } else {
                m_buffer_mgr->HandleSelectOrFocus();
            }
            break;
        }
        case SK_ESC: {
            m_buffer_mgr->Revert();
            break;
        }
        default: {
            break;
        }
        }

        AttachPreeditWithCandidates(request, response);
    }

    void HandleCommit(const Request *request, Response *response) {
        spdlog::debug("HandleCommit");
        m_buffer_mgr->Clear();
        response->set_committed(true);
    }

    void HandleTestSendKey(const Request *request, Response *response) {
        spdlog::debug("HandleTestSendKey");
        if (!m_config->ime_enabled()) {
            // Direct input mode, skip all processing
            response->set_consumable(false);
            return;
        }

        response->set_consumable(true);

        auto const &key_event = request->key_event();
        auto key = key_event.key_code();
        auto const &mods = key_event.modifier_keys();

        if (!mods.empty()) {
            if (mods.size() > 1 || mods.at(0) != MODK_SHIFT) {
                response->set_consumable(false);
            }
        } else if (m_buffer_mgr->IsEmpty() && isgraph(key) == 0) {
            response->set_consumable(false);
        }
    }

    void AttachPreeditWithCandidates(const Request *request, Response *response) {
        auto *preedit = response->mutable_preedit();
        m_buffer_mgr->BuildPreedit(preedit);
        auto *candidate_list = response->mutable_candidate_list();
        m_buffer_mgr->GetCandidates(candidate_list);
        response->set_edit_state(m_buffer_mgr->edit_state());
    }

    void HandleSelectCandidate(const Request *request, Response *response) {
        spdlog::debug("HandleSelectCandidate");
        m_buffer_mgr->SelectCandidate(request->candidate_id());
        AttachPreeditWithCandidates(request, response);
    }

    void HandleFocusCandidate(const Request *request, Response *response) {
        spdlog::debug("HandleFocusCandidate");
        m_buffer_mgr->FocusCandidate(request->candidate_id());
        AttachPreeditWithCandidates(request, response);
    }

    void HandleSetConfig(const Request *request, Response *response) {
        spdlog::debug("HandleSetConfig");
        if (request->has_config()) {
            m_config->UpdateAppConfig(request->config());
            NotifyConfigChangeListeners();
        }
    }

    void NotifyConfigChangeListeners() {
        auto it = m_config_change_listeners.begin();
        while (it != m_config_change_listeners.end()) {
            if (*it != nullptr) {
                (*it)->OnConfigChanged(m_config.get());
                ++it;
            } else {
                it = m_config_change_listeners.erase(it);
            }
        }
    }

    void HandleListEmojis(const Request *request, Response *response) {
        spdlog::debug("HandleListEmojis");
        auto emojis = m_database->GetEmojis();
        auto *candidates = response->mutable_candidate_list();
        for (auto &emoji : emojis) {
            auto *cand = candidates->add_candidates();
            cand->set_id(emoji.category);
            cand->set_value(emoji.value);
        }
    }

    void HandleResetUserData(const Request *request, Response *response) {
        spdlog::debug("HandleResetUserData");
        m_database->ClearNGramsData();
    }

    // void HandleRevert(Command *command, Output *output) {}
    // void HandlePlaceCursor(Command *command, Output *output) {}

    fs::path resource_dir = {};
    std::string m_dbfilename = ":memory:";

    std::unique_ptr<Database> m_database = nullptr;
    std::unique_ptr<BufferMgr> m_buffer_mgr = nullptr;
    std::unique_ptr<KeyConfig> m_keyconfig = nullptr;
    std::unique_ptr<SyllableParser> m_syllable_parser = nullptr;
    std::unique_ptr<Dictionary> m_dictionary = nullptr;
    std::unique_ptr<UserDictionary> m_userdict = nullptr;

    std::vector<std::string> m_valid_syllables;

    std::unordered_map<CommandType, decltype(&EngineImpl::HandleNone)> m_cmd_handlers = {};
    std::vector<ConfigChangeListener *> m_config_change_listeners;
    std::unique_ptr<Config> m_config = nullptr;

    mutable std::mutex m_mutex;
};

} // namespace

std::unique_ptr<Engine> Engine::Create() {
    auto engine = std::make_unique<EngineImpl>();
    engine->Initialize();
    return engine;
}

std::unique_ptr<Engine> Engine::Create(std::string dbfile) {
    auto engine = std::make_unique<EngineImpl>(dbfile);
    engine->Initialize();
    return engine;
}

} // namespace khiin::engine

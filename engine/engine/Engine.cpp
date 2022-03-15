//#ifdef _WIN32
//#ifdef _MSC_VER
//#pragma execution_character_set("utf-8")
//#endif
//#endif

#include "Engine.h"

#include <cstdlib>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

#include "proto/proto.h"
#include "utf8cpp/utf8.h"

#include "BufferMgr.h"
#include "Config.h"
#include "Database.h"
#include "Dictionary.h"
#include "KeyConfig.h"
#include "SyllableParser.h"
#include "utils.h"

namespace khiin::engine {
namespace {

using namespace khiin::proto;
namespace fs = std::filesystem;

class EngineImpl : public Engine {
  public:
    EngineImpl() {}

    EngineImpl(std::string dbfile) : m_dbfilename(dbfile) {}

    void Initialize() {
        auto db_path = fs::path(m_dbfilename);
        if (fs::exists(db_path)) {
            m_database = std::unique_ptr<Database>(Database::Connect(db_path.string()));
        } else {
            m_database = std::unique_ptr<Database>(Database::TestDb());
        }

        m_config = Config::Default();
        m_keyconfig = std::unique_ptr<KeyConfig>(KeyConfig::Create(this));
        m_syllable_parser = std::unique_ptr<SyllableParser>(SyllableParser::Create(this));
        m_dictionary = std::unique_ptr<Dictionary>(Dictionary::Create(this));
        m_buffer_mgr = std::unique_ptr<BufferMgr>(BufferMgr::Create(this));

        m_cmd_handlers[CMD_COMMIT] = &EngineImpl::HandleCommit;
        m_cmd_handlers[CMD_TEST_SEND_KEY] = &EngineImpl::HandleTestSendKey;
        m_cmd_handlers[CMD_SEND_KEY] = &EngineImpl::HandleSendKey;
        m_cmd_handlers[CMD_SELECT_CANDIDATE] = &EngineImpl::HandleSelectCandidate;
        m_cmd_handlers[CMD_FOCUS_CANDIDATE] = &EngineImpl::HandleFocusCandidate;
        m_cmd_handlers[CMD_SET_CONFIG] = &EngineImpl::HandleSetConfig;
        m_cmd_handlers[CMD_LIST_EMOJIS] = &EngineImpl::HandleListEmojis;

        NotifyConfigChangeListeners();
        m_dictionary->Initialize();
    }

    virtual void SendCommand(Command *pCommand) override {
        auto req = pCommand->mutable_request();
        auto res = pCommand->mutable_response();
        decltype(&EngineImpl::HandleNone) handler;

        if (auto it = m_cmd_handlers.find(req->type()); it != m_cmd_handlers.end()) {
            handler = it->second;
        } else {
            handler = &EngineImpl::HandleNone;
        }

        (this->*handler)(pCommand);
    }

    virtual BufferMgr *buffer_mgr() override {
        return m_buffer_mgr.get();
    }

    virtual Database *database() override {
        return m_database.get();
    }

    virtual KeyConfig *keyconfig() override {
        return m_keyconfig.get();
    }

    virtual SyllableParser *syllable_parser() override {
        return m_syllable_parser.get();
    }

    virtual Dictionary *dictionary() override {
        return m_dictionary.get();
    }

    virtual Config *config() override {
        return m_config.get();
    }

    virtual void RegisterConfigChangedListener(ConfigChangeListener *listener) override {
        m_config_change_listeners.push_back(listener);
    }

  private:
    //+---------------------------------------------------------------------------
    //
    // Command-type handlers
    //
    //----------------------------------------------------------------------------

    void HandleNone(Command *command) {}

    void HandleSendKey(Command *command) {
        auto req = command->mutable_request();
        auto res = command->mutable_response();
        auto &key = req->key_event();

        switch (key.special_key()) {
        case SK_NONE: {
            auto key_code = req->key_event().key_code();
            if (isprint(key_code)) {
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
                HandleCommit(command);
                return;
            }
            break;
        }
        case SK_BACKSPACE: {
            if (m_buffer_mgr->IsEmpty()) {
                res->set_consumable(false);
            } else {
                m_buffer_mgr->Erase(CursorDirection::L);
            }
            break;
        }
        case SK_DEL: {
            if (m_buffer_mgr->IsEmpty()) {
                res->set_consumable(false);
            } else {
                m_buffer_mgr->Erase(CursorDirection::R);
            }
            break;
        }
        case SK_SPACE: {
            if (m_buffer_mgr->IsEmpty()) {
                res->set_consumable(false);
            } else {
                m_buffer_mgr->HandleSelectOrFocus();
            }
            break;
        }
        default: {
            break;
        }
        }

        AttachPreeditWithCandidates(command);
    }

    void HandleCommit(Command *command) {
        // command->input().set_type(CommandType::COMMIT);
        auto req = command->mutable_request();
        req->set_type(CMD_COMMIT);
        auto res = command->mutable_response();
        auto preedit = res->mutable_preedit();
        // preedit->set_cursor_position(buffer_mgr->caret_position());
        // auto segment = preedit->add_segments();
        // segment->set_status(SegmentStatus::UNMARKED);
        // segment->set_value(buffer_mgr->getDisplayBuffer());
        m_buffer_mgr->Clear();
    }

    void HandleTestSendKey(Command *command) {
        auto &request = command->request();
        auto res = command->mutable_response();

        if (!m_config->ime_enabled()) {
            // Direct input mode, skip all processing
            res->set_consumable(false);
            return;
        }

        res->set_consumable(true);

        auto &key_event = request.key_event();
        auto key = key_event.key_code();
        auto &mods = key_event.modifier_keys();

        if (!mods.empty()) {
            if (mods.size() > 1 || mods.at(0) != MODK_SHIFT) {
                res->set_consumable(false);
            }
        } else if (m_buffer_mgr->IsEmpty() && !isgraph(key)) {
            res->set_consumable(false);
        }
    }

    void AttachPreeditWithCandidates(Command *command) {
        auto res = command->mutable_response();
        auto preedit = res->mutable_preedit();
        m_buffer_mgr->BuildPreedit(preedit);
        auto candidate_list = command->mutable_response()->mutable_candidate_list();
        m_buffer_mgr->GetCandidates(candidate_list);
        res->set_edit_state(m_buffer_mgr->edit_state());
    }

    void HandleSelectCandidate(Command *command) {
        m_buffer_mgr->SelectCandidate(command->request().candidate_id());
        AttachPreeditWithCandidates(command);
    }

    void HandleFocusCandidate(Command *command) {
        m_buffer_mgr->FocusCandidate(command->request().candidate_id());
        AttachPreeditWithCandidates(command);
    }

    void HandleSetConfig(Command *command) {
        if (command->request().has_config()) {
            m_config->UpdateAppConfig(command->request().config());
            NotifyConfigChangeListeners();
        }
    }

    void NotifyConfigChangeListeners() {
        for (auto &listener : m_config_change_listeners) {
            if (listener) {
                listener->OnConfigChanged(m_config.get());
            }
        }
    }

    void HandleListEmojis(Command *command) {
        auto emojis = m_database->GetEmojis();
        auto candidates = command->mutable_response()->mutable_candidate_list();
        for (auto &emoji : emojis) {
            auto cand = candidates->add_candidates();
            cand->set_id(emoji.category);
            cand->set_value(emoji.value);
        }
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

    std::vector<std::string> m_valid_syllables;

    std::unordered_map<CommandType, decltype(&HandleNone)> m_cmd_handlers = {};
    std::vector<ConfigChangeListener *> m_config_change_listeners;
    std::unique_ptr<Config> m_config = nullptr;
};

} // namespace

Engine *Engine::Create() {
    auto engine = new EngineImpl;
    engine->Initialize();
    return engine;
}

Engine *Engine::Create(std::string dbfile) {
    auto engine = new EngineImpl(dbfile);
    engine->Initialize();
    return engine;
}

} // namespace khiin::engine

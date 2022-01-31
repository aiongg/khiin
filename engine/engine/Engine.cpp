// libtaikey.cpp : Defines the entry point for the application.
//
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

#include "utf8cpp/utf8.h"

#include "BufferMgr.h"
#include "CandidateFinder.h"
#include "Dictionary.h"
#include "KeyConfig.h"
#include "Segmenter.h"
#include "SyllableParser.h"
#include "config.h"
#include "utils.h"

namespace khiin::engine {
namespace {

using namespace khiin::messages;
namespace fs = std::filesystem;

class EngineImpl : public Engine {
  public:
    EngineImpl() {}

    EngineImpl(std::string resource_dir) {
        auto path = fs::path(resource_dir);

        if (fs::exists(path)) {
            this->resource_dir = path;
        }
    }

    void Initialize() {
        if (resource_dir.empty()) {
            resource_dir = Utils::findResourceDirectory();
        }

        if (!resource_dir.empty()) {
            auto db_path = fs::path(resource_dir);
            db_path /= DB_FILE;

            if (fs::exists(db_path)) {
                m_database = std::make_unique<Database>(db_path.string());
            }

            auto cfg_path = fs::path(resource_dir);
            cfg_path /= CONFIG_FILE;

            if (fs::exists(cfg_path)) {
                m_config = std::make_unique<Config>(cfg_path.string());
            }
        }

        if (m_database == nullptr) {
            m_database = std::make_unique<Database>();
        }

        if (m_config == nullptr) {
            m_config = std::make_unique<Config>();
        }

        // m_valid_syllables = m_database->GetSyllableList();
        // m_trie = std::make_unique<Trie>(m_database->GetTrieWordlist(), m_valid_syllables);

        m_keyconfig = std::unique_ptr<KeyConfig>(KeyConfig::Create());
        m_syllable_parser = std::unique_ptr<SyllableParser>(SyllableParser::Create(m_keyconfig.get()));
        m_dictionary = std::unique_ptr<Dictionary>(Dictionary::Create(this));
        // m_splitter = std::make_unique<Splitter>(m_valid_syllables);
        m_segmenter = std::unique_ptr<Segmenter>(Segmenter::Create(this));
        m_candidate_finder = std::unique_ptr<CandidateFinder>(CandidateFinder::Create(this));
        m_buffer_mgr = std::unique_ptr<BufferMgr>(BufferMgr::Create(this));

        command_handlers[CommandType::COMMIT] = &EngineImpl::HandleCommit;
        command_handlers[CommandType::TEST_SEND_KEY] = &EngineImpl::HandleTestSendKey;
        command_handlers[CommandType::SEND_KEY] = &EngineImpl::HandleSendKey;

        m_dictionary->Initialize();
    }

    virtual void SendCommand(Command *pCommand) override {
        auto input = pCommand->mutable_input();
        auto output = pCommand->mutable_output();
        decltype(&EngineImpl::HandleNone) handler;

        if (auto it = command_handlers.find(pCommand->type()); it != command_handlers.end()) {
            handler = it->second;
        } else {
            handler = &EngineImpl::HandleNone;
        }

        (this->*handler)(pCommand);
    }

    virtual BufferMgr *buffer_mgr() override {
        return m_buffer_mgr.get();
    }

    virtual CandidateFinder *candidate_finder() override {
        return m_candidate_finder.get();
    }

    virtual Segmenter *segmenter() override {
        return m_segmenter.get();
    }

    virtual Database *database() override {
        return m_database.get();
    }

    virtual KeyConfig *key_configuration() override {
        return m_keyconfig.get();
    }

    virtual SyllableParser *syllable_parser() override {
        return m_syllable_parser.get();
    }

    virtual Dictionary *dictionary() override {
        return m_dictionary.get();
    }

    virtual Splitter *word_splitter() override {
        return m_dictionary->word_splitter();
    }

  private:
    //+---------------------------------------------------------------------------
    //
    // Command-type handlers
    //
    //----------------------------------------------------------------------------

    void HandleNone(Command *command) {}

    void HandleSendKey(Command *command) {
        auto input = command->mutable_input();
        auto output = command->mutable_output();
        auto &key = input->key_event();

        switch (key.special_key()) {
        case SpecialKey::SK_NONE: {
            auto key_code = input->key_event().key_code();
            if (isalnum(key_code) || key_code == '-') {
                m_buffer_mgr->Insert(static_cast<char>(key_code));
            }
            break;
        }
        case SpecialKey::SK_RIGHT: {
            m_buffer_mgr->MoveCaret(CursorDirection::R);
            break;
        }
        case SpecialKey::SK_LEFT: {
            m_buffer_mgr->MoveCaret(CursorDirection::L);
            break;
        }
        case SpecialKey::SK_ENTER: {
            HandleCommit(command);
            return;
        }
        case SpecialKey::SK_BACKSPACE: {
            if (m_buffer_mgr->IsEmpty()) {
                output->set_consumable(false);
            } else {
                m_buffer_mgr->Erase(CursorDirection::L);
            }
            break;
        }
        case SpecialKey::SK_DEL: {
            if (m_buffer_mgr->IsEmpty()) {
                output->set_consumable(false);
            } else {
                m_buffer_mgr->Erase(CursorDirection::R);
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
        command->set_type(CommandType::COMMIT);
        auto input = command->mutable_input();
        auto output = command->mutable_output();
        auto preedit = output->mutable_preedit();
        // preedit->set_cursor_position(buffer_mgr->caret_position());
        // auto segment = preedit->add_segments();
        // segment->set_status(SegmentStatus::UNMARKED);
        // segment->set_value(buffer_mgr->getDisplayBuffer());
        m_buffer_mgr->Clear();
    }

    void HandleTestSendKey(Command *command) {
        auto input = command->mutable_input();
        auto output = command->mutable_output();

        if (m_buffer_mgr->IsEmpty() && !isalnum(input->key_event().key_code())) {
            output->set_consumable(false);
        } else {
            output->set_consumable(true);
        }
    }

    void AttachPreeditWithCandidates(Command *command) {
        auto output = command->mutable_output();
        auto preedit = output->mutable_preedit();
        m_buffer_mgr->BuildPreedit(preedit);
        // preedit->set_cursor_position(buffer->getCursor());
        // auto segment = preedit->add_segments();
        // segment->set_status(SegmentStatus::COMPOSING);
        // segment->set_value(buffer->getDisplayBuffer());

        auto candidate_list = command->mutable_output()->mutable_candidate_list();
        m_buffer_mgr->GetCandidates(candidate_list);

        // auto cand_list = output->mutable_candidate_list();
        // for (auto &c : curr_candidates) {
        //    auto candidate = cand_list->add_candidates();
        //    candidate->set_value(c.text);
        //    candidate->set_category(Candidate_Category_BASIC);
        //}
    }

    // void HandleRevert(Command *command, Output *output) {}
    // void HandleSelectCandidate(Command *command, Output *output) {}
    // void HandleFocusCandidate(Command *command, Output *output) {}
    // void HandlePlaceCursor(Command *command, Output *output) {}

    fs::path resource_dir = {};

    std::unique_ptr<Database> m_database = nullptr;
    std::unique_ptr<Config> m_config = nullptr;
    std::unique_ptr<Splitter> m_splitter = nullptr;
    std::unique_ptr<Trie> m_trie = nullptr;
    std::unique_ptr<BufferMgr> m_buffer_mgr = nullptr;
    std::unique_ptr<CandidateFinder> m_candidate_finder = nullptr;
    std::unique_ptr<Segmenter> m_segmenter = nullptr;
    std::unique_ptr<KeyConfig> m_keyconfig = nullptr;
    std::unique_ptr<SyllableParser> m_syllable_parser = nullptr;
    std::unique_ptr<Dictionary> m_dictionary = nullptr;

    string_vector m_valid_syllables;

    std::shared_ptr<Output> prev_output = nullptr;
    std::unordered_map<CommandType, decltype(&HandleNone)> command_handlers = {};
};

} // namespace

Engine *Engine::Create() {
    auto engine = new EngineImpl;
    engine->Initialize();
    return engine;
}

Engine *Engine::Create(std::string home_dir) {
    auto engine = new EngineImpl(home_dir);
    engine->Initialize();
    return engine;
}

} // namespace khiin::engine

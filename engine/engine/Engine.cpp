// libtaikey.cpp : Defines the entry point for the application.
//
#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#include <cstdlib>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

//#include <boost/format.hpp>
//#include <boost/log/trivial.hpp>

#include "utf8cpp/utf8.h"

#include "Engine.h"
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
                database = std::make_unique<Database>(db_path.string());
            }

            auto cfg_path = fs::path(resource_dir);
            cfg_path /= CONFIG_FILE;

            if (fs::exists(cfg_path)) {
                config = std::make_unique<Config>(cfg_path.string());
            }
        }

        if (database == nullptr) {
            database = std::make_unique<Database>();
        }

        if (config == nullptr) {
            config = std::make_unique<Config>();
        }

        auto syllableList = database->GetSyllableList();
        splitter = std::make_unique<Splitter>(syllableList);
        trie = std::make_unique<Trie>(database->GetTrieWordlist(), syllableList);
        candidateFinder = std::make_unique<CandidateFinder>(database.get(), splitter.get(), trie.get());
        buffer_mgr = std::unique_ptr<BufferManager>(BufferManager::Create(candidateFinder.get()));

        command_handlers[CommandType::COMMIT] = &EngineImpl::HandleCommit;
        command_handlers[CommandType::TEST_SEND_KEY] = &EngineImpl::HandleTestSendKey;
        command_handlers[CommandType::SEND_KEY] = &EngineImpl::HandleSendKey;
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
                buffer_mgr->Insert(static_cast<char>(key_code));
            }
            break;
        }
        case SpecialKey::SK_RIGHT: {
            buffer_mgr->MoveCaret(CursorDirection::R);
            break;
        }
        case SpecialKey::SK_LEFT: {
            buffer_mgr->MoveCaret(CursorDirection::L);
            break;
        }
        case SpecialKey::SK_ENTER: {
            HandleCommit(command);
            return;
        }
        case SpecialKey::SK_BACKSPACE: {
            if (buffer_mgr->IsEmpty()) {
                output->set_consumable(false);
            } else {
                buffer_mgr->Erase(CursorDirection::L);
            }
            break;
        }
        case SpecialKey::SK_DEL: {
            if (buffer_mgr->IsEmpty()) {
                output->set_consumable(false);
            } else {
                buffer_mgr->Erase(CursorDirection::R);
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
        preedit->set_cursor_position(buffer_mgr->getCursor());
        auto segment = preedit->add_segments();
        segment->set_status(SegmentStatus::UNMARKED);
        segment->set_value(buffer_mgr->getDisplayBuffer());
        buffer_mgr->Clear();
    }

    void HandleTestSendKey(Command *command) {
        auto input = command->mutable_input();
        auto output = command->mutable_output();

        if (buffer_mgr->IsEmpty() && !isalnum(input->key_event().key_code())) {
            output->set_consumable(false);
        } else {
            output->set_consumable(true);
        }
    }

    void AttachPreeditWithCandidates(Command *command) {
        auto output = command->mutable_output();
        auto preedit = output->mutable_preedit();
        buffer_mgr->BuildPreedit(preedit);
        // preedit->set_cursor_position(buffer->getCursor());
        // auto segment = preedit->add_segments();
        // segment->set_status(SegmentStatus::COMPOSING);
        // segment->set_value(buffer->getDisplayBuffer());

        auto curr_candidates = buffer_mgr->getCandidates();
        auto cand_list = output->mutable_candidate_list();
        for (auto &c : curr_candidates) {
            auto candidate = cand_list->add_candidates();
            candidate->set_value(c.text);
            candidate->set_category(Candidate_Category_BASIC);
        }
    }

    // void HandleRevert(Command *command, Output *output) {}
    // void HandleSelectCandidate(Command *command, Output *output) {}
    // void HandleFocusCandidate(Command *command, Output *output) {}
    // void HandlePlaceCursor(Command *command, Output *output) {}

    fs::path resource_dir = {};
    std::unique_ptr<Database> database = nullptr;
    std::unique_ptr<Config> config = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;
    std::unique_ptr<Trie> trie = nullptr;
    std::unique_ptr<BufferManager> buffer_mgr = nullptr;
    std::unique_ptr<CandidateFinder> candidateFinder = nullptr;
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

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

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

#include "utf8cpp/utf8.h"

#include "engine.h"
#include "utils.h"

namespace khiin::engine {

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
                database = std::make_unique<TKDB>(db_path.string());
            }

            auto cfg_path = fs::path(resource_dir);
            cfg_path /= CONFIG_FILE;

            if (fs::exists(cfg_path)) {
                config = std::make_unique<Config>(cfg_path.string());
            }
        }

        if (database == nullptr) {
            database = std::make_unique<TKDB>();
        }

        if (config == nullptr) {
            config = std::make_unique<Config>();
        }

        auto syllableList = database->selectSyllableList();
        splitter = std::make_unique<Splitter>(syllableList);
        trie = std::make_unique<Trie>(database->selectTrieWordlist(), syllableList);
        candidateFinder = std::make_unique<CandidateFinder>(database.get(), splitter.get(), trie.get());
        buffer = std::make_unique<BufferManager>(candidateFinder.get());

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

        (this->*handler)(input, output);
    }

  private:
    void HandleNone(Input *input, Output *output) {}

    void HandleSendKey(Input *input, Output *output) {
        auto &key = input->key_event();

        switch (key.special_key()) {
        case SpecialKey::SK_NONE: {
            auto key_code = input->key_event().key_code();
            if (isalnum(key_code)) {
                buffer->insert(static_cast<char>(key_code));
            }
            break;
        }
        case SpecialKey::SK_RIGHT: {
            buffer->moveCursor(CursorDirection::R);
            break;
        }
        case SpecialKey::SK_LEFT: {
            buffer->moveCursor(CursorDirection::L);
            break;
        }
        case SpecialKey::SK_ENTER: {
            buffer->clear();
            break;
        }
        case SpecialKey::SK_BACKSPACE: {
            if (buffer->empty()) {
                output->set_consumable(false);
            } else {
                buffer->erase(CursorDirection::L);
            }
            break;
        }
        case SpecialKey::SK_DEL: {
            if (buffer->empty()) {
                output->set_consumable(false);
            } else {
                buffer->erase(CursorDirection::R);
            }
            break;
        }
        default: {
            break;
        }
        }

        auto segment = output->mutable_composition()->add_segments();
        segment->set_status(SegmentStatus::COMPOSING);
        segment->set_value(buffer->getDisplayBuffer());

        auto curr_candidates = buffer->getCandidates();
        auto cand_list = output->mutable_candidate_list();
        for (auto &c : curr_candidates) {
            auto candidate = cand_list->add_candidates();
            candidate->set_value(c.text);
            candidate->set_category(Candidate_Category_NORMAL);
        }
    }

    void HandleCommit(Input *input, Output *output) {
        auto segment = output->mutable_composition()->add_segments();
        segment->set_status(SegmentStatus::COMPOSING);
        segment->set_value(buffer->getDisplayBuffer());
        buffer->clear();
    }

    void HandleTestSendKey(Input *input, Output *output) {
        if (!buffer->empty() || isalnum(input->key_event().key_code())) {
            output->set_consumable(true);
        } else {
            output->set_consumable(false);
        }
    }

    // void HandleRevert(Command *command, Output *output) {}
    // void HandleSelectCandidate(Command *command, Output *output) {}
    // void HandleFocusCandidate(Command *command, Output *output) {}
    // void HandlePlaceCursor(Command *command, Output *output) {}

    fs::path resource_dir = {};
    std::unique_ptr<TKDB> database = nullptr;
    std::unique_ptr<Config> config = nullptr;
    std::unique_ptr<Splitter> splitter = nullptr;
    std::unique_ptr<Trie> trie = nullptr;
    std::unique_ptr<BufferManager> buffer = nullptr;
    std::unique_ptr<CandidateFinder> candidateFinder = nullptr;
    std::unordered_map<CommandType, decltype(&HandleNone)> command_handlers = {};
    std::shared_ptr<Output> prev_output = nullptr;
};

Engine *EngineFactory::Create() {
    auto engine = new EngineImpl;
    engine->Initialize();
    return engine;
}

Engine *EngineFactory::Create(std::string home_dir) {
    auto engine = new EngineImpl(home_dir);
    engine->Initialize();
    return engine;
}

} // namespace khiin::engine

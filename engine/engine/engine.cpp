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
        case KeyEvent_SpecialKey_NONE: {
            auto key_code = input->key_event().key_code();
            if (isalnum(key_code)) {
                buffer->insert(static_cast<char>(key_code));
            }
            break;
        }
        case KeyEvent_SpecialKey_RIGHT: {
            buffer->moveCursor(CursorDirection::R);
            break;
        }
        case KeyEvent_SpecialKey_LEFT: {
            buffer->moveCursor(CursorDirection::L);
            break;
        }
        case KeyEvent_SpecialKey_ENTER: {
            buffer->clear();
            break;
        }
        case KeyEvent_SpecialKey_BACKSPACE: {
            if (buffer->empty()) {
                output->set_consumable(false);
            } else {
                buffer->erase(CursorDirection::L);
            }
            break;
        }
        case KeyEvent_SpecialKey_DEL: {
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
        segment->set_status(Composition_Segment_Status_COMPOSING);
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
        segment->set_status(Composition_Segment_Status_COMMITTED);
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

    //void HandleRevert(Command *command, Output *output) {}
    //void HandleSelectCandidate(Command *command, Output *output) {}
    //void HandleFocusCandidate(Command *command, Output *output) {}
    //void HandlePlaceCursor(Command *command, Output *output) {}

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

// taikey::Engine Public Members

/**
 * Engine() default constructor.
 *
 * Important! Only call this method when the TAIKEY_PATH
 * environment variable has been set appropriately through
 * the operating system. The variable should be set to a list
 * of paths to check in order for the database and config files.
 *
 * If you haven't set the TAIKEY_PATH variable, you will only get
 * an empty, in-memory database. (Maybe useful for testing only)
 *
 * If you haven't set TAIKEY_PATH, you may pass the resource
 * directory location directly to the Engine(std::string) constructor.
 */
// Engine::Engine() {
//    auto resourceDir = Utils::findResourceDirectory();
//
//    if (!resourceDir.empty()) {
//        auto dbfile = fs::path(resourceDir);
//        dbfile /= DB_FILE;
//
//        if (fs::exists(dbfile)) {
//            database = std::make_unique<TKDB>(dbfile.string());
//        }
//
//        auto configfile = fs::path(resourceDir);
//        configfile /= CONFIG_FILE;
//
//        if (fs::exists(configfile)) {
//            config = std::make_unique<Config>(configfile.string());
//        }
//    }
//
//    if (database == nullptr) {
//        database = std::make_unique<TKDB>();
//    }
//
//    if (config == nullptr) {
//        config = std::make_unique<Config>();
//    }
//
//    auto syllableList = database->selectSyllableList();
//    splitter = std::make_unique<Splitter>(syllableList);
//    trie = std::make_unique<Trie>(database->selectTrieWordlist(), syllableList);
//    candidateFinder = std::make_unique<CandidateFinder>(
//        database.get(), splitter.get(), trie.get());
//    buffer = std::make_unique<BufferManager>(candidateFinder.get());
//}
//
// Engine::Engine(std::string resourceDir) {
//    if (!resourceDir.empty()) {
//        auto dbfile = fs::path(resourceDir);
//        dbfile /= DB_FILE;
//
//        if (fs::exists(dbfile)) {
//            database = std::make_unique<TKDB>(dbfile.string());
//        }
//
//        auto configfile = fs::path(resourceDir);
//        configfile /= CONFIG_FILE;
//
//        if (fs::exists(configfile)) {
//            config = std::make_unique<Config>(configfile.string());
//        }
//    }
//
//    if (database == nullptr) {
//        database = std::make_unique<TKDB>();
//    }
//
//    if (config == nullptr) {
//        config = std::make_unique<Config>();
//    }
//
//    auto syllableList = database->selectSyllableList();
//    splitter = std::make_unique<Splitter>(syllableList);
//    trie = std::make_unique<Trie>(database->selectTrieWordlist(), syllableList);
//    candidateFinder = std::make_unique<CandidateFinder>(
//        database.get(), splitter.get(), trie.get());
//    buffer = std::make_unique<BufferManager>(candidateFinder.get());
//}

/*
auto Engine::TestConsumable(KeyCode c) -> bool {
    if (IsAlphaNumeric(c) || c == KeyCode::HYPHEN) {
        return true;
    }
    return false;
}

auto Engine::focusCandidate(size_t index, ImeDisplayData &data) -> RetVal {
    auto success = buffer->focusCandidate(index);

    return RetVal::TODO;
}

auto Engine::onKeyDown(KeyCode kc, ImeDisplayData &data) -> RetVal {
    if (IsAlphaNumeric(kc)) {
        auto ch = static_cast<char>(kc);
        buffer->insert(ch);
    } else {
        switch (kc) {
        case KeyCode::ENTER:
            buffer->clear();
            break;
        case KeyCode::BACK:
            if (buffer->empty()) {
                return RetVal::NotConsumed;
            }

            buffer->erase(CursorDirection::L);
            if (buffer->empty()) {
                data.buffer.clear();
                data.cursor = 0;
                data.candidates.clear();
                return RetVal::Cancelled;
            }
            break;
        case KeyCode::LEFT:
            buffer->moveCursor(CursorDirection::L);
            break;
        case KeyCode::RIGHT:
            buffer->moveCursor(CursorDirection::R);
            break;
        case KeyCode::DEL:
            buffer->erase(CursorDirection::R);
            break;
        }
    }

    data.buffer = buffer->getDisplayBuffer();
    data.cursor = static_cast<int>(buffer->getCursor());
    data.candidates = buffer->getCandidates();
    // ENTER, LEFT/RIGHT
    return RetVal::Consumed;
}

EngineState Engine::getState() const {
    return engineState_;
}

std::string Engine::getBuffer() const {
    return buffer->getDisplayBuffer();
    // return normalize(keyBuffer_, norm_nfc);
}

RetVal Engine::Reset() {
    buffer->clear();
    return RetVal::OK;
}

// key handlers

RetVal Engine::onKeyDown_(KeyCode keyCode) {
    KeyHandlerFn handler{};

    switch (inputMode_) {
    case InputMode::Pro:
        handler = &Engine::onKeyDownPro_;
        break;
    case InputMode::Normal:
        switch (engineState_) {
        case EngineState::Ready:
            handler = &Engine::handleKeyOnReady_;
            break;
        case EngineState::Editing:
            handler = &Engine::handleEditing_;
            break;
        case EngineState::BufferBySegment:
            handler = &Engine::handleNavBySegment_;
            break;
        case EngineState::BufferByLetter:
            handler = &Engine::handleNavByLetter_;
            break;
        case EngineState::ChoosingCandidate:
            handler = &Engine::handleChoosingCandidate_;
            break;
        }
    }

    if (handler)
        return (this->*handler)(keyCode);

    return

        RetVal::NotConsumed;
}

// Pro Mode key handler methods

RetVal Engine::onKeyDownPro_(KeyCode keyCode) {
    return RetVal::NotConsumed;
}

// Normal Mode key handlers

RetVal Engine::handleKeyOnReady_(KeyCode keyCode) {
    return setEngineState_(EngineState::Editing, keyCode);
}

RetVal Engine::handleEditing_(KeyCode keyCode) {
    switch (keyCode) {
    case KeyCode::TAB:
    case KeyCode::DOWN:
        // switch to ByCandidates
        return RetVal::TODO;
    case KeyCode::SPACE:
        // switch to BySegment
        return RetVal::TODO;
    }

    if (isalnum((char)keyCode)) {
        if (toneKeys_ == ToneKeys::Telex) {
        }
    }

    return RetVal::TODO;
}

RetVal Engine::handleChoosingCandidate_(KeyCode keyCode) {
    // switch (keyCode) {}
    return RetVal::TODO;
}

RetVal Engine::handleNavByLetter_(KeyCode keyCode) {
    // switch (keyCode) {}
    return RetVal::TODO;
}

RetVal Engine::handleNavBySegment_(KeyCode keyCode) {
    switch (keyCode) {
    case KeyCode::RIGHT:
        return RetVal::Consumed;
    case KeyCode::LEFT:
        return RetVal::Consumed;
    default:
        return RetVal::TODO;
    }
}

RetVal Engine::setEngineState_(EngineState nextEngineState, KeyCode keyCode) {
    if (handleStateTransition_(engineState_, nextEngineState, keyCode) == RetVal::OK) {
        engineState_ = nextEngineState;
    }

    return RetVal::TODO;
}

RetVal Engine::handleStateTransition_(EngineState prev, EngineState next, KeyCode keyCode) {
    switch (prev) {
    case EngineState::Ready:
        switch (next) {
        case EngineState::Editing:
            return handleEditing_(keyCode);
        }
    case EngineState::BufferBySegment:
        switch (next) {
        case EngineState::BufferByLetter:
            bySegmentToByLetter_(keyCode);
            break;
        }
        break;
    }
    return RetVal::TODO;
}

RetVal Engine::bySegmentToByLetter_(KeyCode keyCode) {
    return handleNavByLetter_(keyCode);
}
*/

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

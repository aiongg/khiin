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

#include <utf8cpp/utf8.h>

#include "engine.h"
#include "utils.h"

namespace taikey {

namespace fs = std::filesystem;

enum class Special {
    NasalCombo,
    NasalSolo,
    OUCombo,
    OUSolo,
};

std::unordered_map<Special, char> SPEC_KEY_MAP = {{Special::NasalCombo, 'n'},
                                                  {Special::OUCombo, 'u'}};

// TaiKey::Engine Public Members

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
Engine::Engine() {
    auto resourceDir = Utils::findResourceDirectory();

    if (!resourceDir.empty()) {
        auto dbfile = fs::path(resourceDir);
        dbfile /= DB_FILE;

        if (fs::exists(dbfile)) {
            database = std::make_unique<TKDB>(dbfile.string());
        }

        auto configfile = fs::path(resourceDir);
        configfile /= CONFIG_FILE;

        if (fs::exists(configfile)) {
            config = std::make_unique<Config>(configfile.string());
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
    candidateFinder = std::make_unique<CandidateFinder>(
        database.get(), splitter.get(), trie.get());
    buffer = std::make_unique<BufferManager>(candidateFinder.get());
}

Engine::Engine(std::string resourceDir) {
    if (!resourceDir.empty()) {
        auto dbfile = fs::path(resourceDir);
        dbfile /= DB_FILE;

        if (fs::exists(dbfile)) {
            database = std::make_unique<TKDB>(dbfile.string());
        }

        auto configfile = fs::path(resourceDir);
        configfile /= CONFIG_FILE;

        if (fs::exists(configfile)) {
            config = std::make_unique<Config>(configfile.string());
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
    candidateFinder = std::make_unique<CandidateFinder>(
        database.get(), splitter.get(), trie.get());
    buffer = std::make_unique<BufferManager>(candidateFinder.get());
}

auto Engine::consumable(KeyCode c) -> bool { return true; }

auto Engine::focusCandidate(size_t index, ImeDisplayData &data) -> RetVal {
    auto success = buffer->focusCandidate(index);

    return RetVal::TODO;
}

auto Engine::onKeyDown(KeyCode kc, ImeDisplayData &data) -> RetVal {
    if (KeyCode::D0 <= kc && kc <= KeyCode::D9 ||
        KeyCode::A <= kc && kc <= KeyCode::Z ||
        KeyCode::A_LC <= kc && kc <= KeyCode::Z_LC) {
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

EngineState Engine::getState() const { return engineState_; }

std::string Engine::getBuffer() const {
    return buffer->getDisplayBuffer();
    // return normalize(keyBuffer_, norm_nfc);
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
    if (handleStateTransition_(engineState_, nextEngineState, keyCode) ==
        RetVal::OK) {
        engineState_ = nextEngineState;
    }

    return RetVal::TODO;
}

RetVal Engine::handleStateTransition_(EngineState prev, EngineState next,
                                      KeyCode keyCode) {
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

} // namespace TaiKey

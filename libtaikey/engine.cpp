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

namespace TaiKey {

namespace fs = std::filesystem;

enum class Special {
    NasalCombo,
    NasalSolo,
    OUCombo,
    OUSolo,
};

std::unordered_map<Special, char> SPEC_KEY_MAP = {{Special::NasalCombo, 'n'},
                                                  {Special::OUCombo, 'u'}};

auto splitString(std::string_view str, char delimiter) {
    auto ret = VStr();
    auto begin = size_t(0);
    auto end = str.find(delimiter);
    while (end != std::string::npos) {
        ret.emplace_back(str.substr(begin, end - begin));
        begin = end + 1;
        end = str.find(delimiter, begin);
    }
    ret.emplace_back(str.substr(begin, end));
    return ret;
}

auto findResourceDirectory() {
#pragma warning(push)
#pragma warning(disable : 4996)
    auto tkpath = ::getenv(TaiKeyPath.c_str());
#pragma warning(pop)

    if (tkpath == nullptr) {
        return fs::path();
    }

#ifdef _WIN32
    auto searchDirectories = splitString(tkpath, ';');
#else
    auto searchDirectories = splitString(tkpath, ':');
#endif
    fs::path path;
    for (auto &dir : searchDirectories) {
        path = fs::path(dir) /= DB_FILE;
        if (fs::exists(path)) {
            return fs::path(dir);
        }
    }

    return fs::path();
}

Engine::Engine() {
    auto resourceDir = findResourceDirectory();

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

// public members

void Engine::reset() { engineState_ = EngineState::Ready; }

RetVal Engine::onKeyDown(char c) { return RetVal::Consumed; }

RetVal Engine::onKeyDown(KeyCode keyCode) {
    BOOST_LOG_TRIVIAL(debug) << boost::format("onKeyDown(%1%)") % (char)keyCode;

    return onKeyDown_(keyCode);
}

EngineState Engine::getState() const { return engineState_; }

std::string Engine::getBuffer() const {
    // return normalize(keyBuffer_, norm_nfc);
    return std::string();
}

// private members

int Engine::getDisplayBufferLength_() {
    int len = 0;

    // for (int i = 0; i < displayBuffer_.segmentCount; i++) {
    //    if (i == displayBuffer_.selectedSegment &&
    //        engineState_ == EngineState::BufferByLetter) {
    //        len += displayBuffer_.segments[i].inputTextLength;
    //    } else {
    //        len += displayBuffer_.segments[i].displayTextLength;
    //    }
    //}

    return len;
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
    // char c = (char)keyCode;

    // if (keyBuffer_.empty()) {
    //    keyBuffer_.push_back(c);
    //    return TK_CONSUMED;
    //}

    // if (TONE_KEY_MAP.find(c) != TONE_KEY_MAP.end()) {
    //    string tone = _getToneOfKey(c);
    //    keyBuffer_ = normalize(placeToneOnSyllable(keyBuffer_, tone),
    //    norm_nfc);
    //} else if (keyBuffer_.back() == 'o' &&
    //           c == SPEC_KEY_MAP.at(Special::OUCombo)) {
    //    keyBuffer_ += u8"\u0358";
    //} else if (keyBuffer_.back() == 'n' &&
    //           c == SPEC_KEY_MAP.at(Special::NasalCombo)) {
    //    replace_last(keyBuffer_, u8"n", u8"\u207f");
    //} else if (c == '\b') {
    //    popBack_();
    //} else {
    //    keyBuffer_.push_back(c);
    //}

    return RetVal::NotConsumed;
}

// Normal Mode key handlers

RetVal Engine::handleKeyOnReady_(KeyCode keyCode) {
    return setEngineState_(EngineState::Editing, keyCode);
}

RetVal Engine::handleEditing_(KeyCode keyCode) {
    // if (displayBuffer_.segmentCount == 0) {
    //    if (!isalpha((char)keyCode)) {
    //        return TK_NOT_CONSUMED;
    //    }

    //    displayBuffer_.selectedSegment = 0;
    //    displayBuffer_.cursorPosition = 1;
    //    displayBuffer_.segmentCount = 1;
    //    displayBuffer_.displaySegmentOffsets[0] = 1;

    //    displayBuffer_.segments[0].rawText.push_back((char)keyCode);
    //    displayBuffer_.segments[0].rawTextLength = 1;
    //    displayBuffer_.segments[0].numberText.push_back((char)keyCode);
    //    displayBuffer_.segments[0].numberTextLength = 1;
    //    displayBuffer_.segments[0].inputText.push_back((char)keyCode);
    //    displayBuffer_.segments[0].inputTextLength = 1;
    //    displayBuffer_.segments[0].displayText.push_back((char)keyCode);
    //    displayBuffer_.segments[0].displayTextLength = 1;

    //    return TK_CONSUMED;
    //}

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

        // raw buffer + char
        // DisplayBufferSegment curr =
        //    displayBuffer_.segments[displayBuffer_.selectedSegment];

        // curr.rawText += (char)keyCode;

        // calculate new display buffer & cursor
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
        //    if (displayBuffer_.selectedSegment + 1 <
        //    displayBuffer_.segmentCount)
        //    {
        //        displayBuffer_.selectedSegment++;
        //    }
        return RetVal::Consumed;
    case KeyCode::LEFT:
        //    if (displayBuffer_.selectedSegment > 0) {
        //        displayBuffer_.selectedSegment--;
        //    } else {
        //        setEngineState_(EngineState::BufferByLetter, keyCode);
        //    }
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

    // TODO
    return RetVal::TODO;
}

RetVal Engine::bySegmentToByLetter_(KeyCode keyCode) {
    return handleNavByLetter_(keyCode);
}

} // namespace TaiKey

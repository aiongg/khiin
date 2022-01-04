// libtaikey.cpp : Defines the entry point for the application.
//
#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <utf8cpp/utf8.h>

#include "libtaikey.h"

namespace TaiKey {

std::string defaultSettings = R"settings({
    "T2": "s"
})settings";

struct taikey_settings {
    // char key_T2;
    // void load(const string &filename);
};

/*
void taikey_settings::load(const string &filename = "") {
    pt::ptree tree;
    if (filename == "") {
        stringstream ss;
        ss << defaultSettings;
        pt::read_json(ss, tree);
    }
    else {

    }

    key_T2 = tree.get<char>("T2");
}
*/

enum class Special {
    NasalCombo,
    NasalSolo,
    OUCombo,
    OUSolo,
};

std::unordered_map<Special, char> SPEC_KEY_MAP = {{Special::NasalCombo, 'n'},
                                                  {Special::OUCombo, 'u'}};

// string _getToneOfKey(const char c) {
//    auto t = TONE_KEY_MAP.find(c);
//    if (t == TONE_KEY_MAP.end()) {
//        return "";
//    }
//
//    auto s = ToneToUtf8Map.find(t->second);
//    if (s == ToneToUtf8Map.end()) {
//        return "";
//    }
//
//    return s->second;
//}

// input is a string with numeric tones
// std::string inputTelexKeyToNumericText(std::string input, char key) {
//    if (input.size() < 1) {
//        input.push_back(key);
//        return input;
//    }
//
//    char end = input.back();
//
//    if (isdigit(end)) {
//        input.push_back(key);
//        return input;
//    }
//
//    Tone tone = Tone::NaT;
//
//    for (const auto &it : TELEX_KEYS) {
//        if (it.second == key) {
//            tone = it.first;
//            break;
//        }
//    }
//
//    if ((PTKH.find(end) != PTKH.end()) && tone == Tone::T8) {
//        tone = Tone::T7;
//    } else if ((PTKH.find(end) == PTKH.end() && tone == Tone::T7)) {
//        tone = Tone::T8;
//    }
//
//    input.push_back(ToneToDigitMap[tone]);
//    return input;
//}

/**
 * TKEngine::
 */

// constructor

Engine::Engine() {
    // if (!READY) {
    //    initialize();
    //}

    //init_locale();

    inputMode_ = InputMode::Pro;
    toneKeys_ = ToneKeys::Numeric;

    keyBuffer_.reserve(10);
    reset();
}

// public members

void Engine::reset() {
    keyBuffer_.clear();
    engineState_ = EngineState::Ready;
}

RetVal Engine::onKeyDown(char c) { return onKeyDown_((KeyCode)c); }

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

void Engine::popBack_() {
    if (keyBuffer_.empty()) {
        return;
    }

    keyBuffer_.pop_back();

    while (!utf8::is_valid(keyBuffer_.begin(), keyBuffer_.end())) {
        keyBuffer_.pop_back();
    }
}

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

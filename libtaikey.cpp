// libtaikey.cpp : Defines the entry point for the application.
//
#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#include <unordered_map>

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
#include <boost/regex.hpp>

// #include <unicode/uclean.h>
#include <utf8cpp/utf8.h>

#include "libtaikey.h"

using namespace std;
using namespace boost;
using namespace boost::locale;

namespace TaiKey {

string defaultSettings = R"settings({
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

enum class Tone {
    NaT,
    T2,
    T3,
    T5,
    T6,
    T7,
    T8,
    T9,
    TK,
};

enum class Special {
    NasalCombo,
    NasalSolo,
    OUCombo,
    OUSolo,
};

unordered_map<char, Tone> TONE_KEY_MAP = {
    {'s', Tone::T2}, {'f', Tone::T3}, {'l', Tone::T5}, {'j', Tone::T7},
    {'y', Tone::T8}, {'w', Tone::T9}, {'q', Tone::TK}};

unordered_map<Tone, string> TONE_UTF_MAP = {
    {Tone::T2, u8"\u0301"}, {Tone::T3, u8"\u0300"}, {Tone::T5, u8"\u0302"},
    {Tone::T6, u8"\u030c"}, {Tone::T7, u8"\u0304"}, {Tone::T8, u8"\u030d"},
    {Tone::T9, u8"\u0306"}, {Tone::TK, u8"\u00b7"}};

unordered_map<Special, char> SPEC_KEY_MAP = {{Special::NasalCombo, 'n'},
                                             {Special::OUCombo, 'u'}};

string TONES[] = {u8"\u0301", u8"\u0300", u8"\u0302", u8"\u030c",
                  u8"\u0304", u8"\u030d", u8"\u0306", u8"\u00b7"};

string _getToneOfKey(const char c) {
    auto t = TONE_KEY_MAP.find(c);
    if (t == TONE_KEY_MAP.end()) {
        return "";
    }

    auto s = TONE_UTF_MAP.find(t->second);
    if (s == TONE_UTF_MAP.end()) {
        return "";
    }

    return s->second;
}

string stripDiacritics(const string &s) {
    string sRet = normalize(s, norm_nfd);
    for (string tone : TONES) {
        size_t found = sRet.find(tone);
        if (found != string::npos) {
            sRet.erase(found, tone.size());
            break;
        }
    }
    return sRet;
}

// s must be one syllable
string placeToneOnSyllable(const string &syllable, const string &tone) {
    static regex e(u8"o[ae][mnptkh]");
    static string ordered_vowel_matches[] = {u8"o", u8"a",  u8"e", u8"u",
                                             u8"i", u8"ng", u8"m"};

    string sRet = stripDiacritics(syllable);

    BOOST_LOG_TRIVIAL(debug) << boost::format("stripped syl: %1%") % sRet;

    smatch match;
    size_t found;

    if (regex_search(sRet, match, e)) {
        found = match.position() + 1;
    } else {
        for (string v : ordered_vowel_matches) {
            found = sRet.find(v);

            if (found != string::npos) {
                break;
            }
        }
    }

    if (found == string::npos) {
        return syllable;
    }

    sRet.insert(found + 1, tone);

    return sRet;
}

TKEngine::TKEngine() {
    if (!READY) {
        initialize();
    }

    inputMode_ = InputMode::Pro;

    keyBuffer_.reserve(10);
    reset();
}

void TKEngine::reset() {
    keyBuffer_.clear();
    engineState_ = EngineState::Ready;
}

bool TKEngine::onKeyDown(char c) { return onKeyDown_((KeyCode)c); }

bool TKEngine::onKeyDown(KeyCode keyCode) {
    BOOST_LOG_TRIVIAL(debug) << boost::format("onKeyDown(%1%)") % (char)keyCode;

    return onKeyDown_(keyCode);
}

bool TKEngine::onKeyDown_(KeyCode keyCode) {
    KeyHandlerFn handler{};

    switch (inputMode_) {
    case InputMode::Pro:
        handler = &TKEngine::onKeyDownPro_;
        break;
    case InputMode::Normal:
        switch (engineState_) {
        case EngineState::Ready:
            handler = &TKEngine::handleKeyOnReady_;
            break;
        case EngineState::Editing:
            handler = &TKEngine::handleEditing_;
            break;
        case EngineState::BufferBySegment:
            handler = &TKEngine::handleNavBySegment_;
            break;
        case EngineState::BufferByLetter:
            handler = &TKEngine::handleNavByLetter_;
            break;
        case EngineState::ChoosingCandidate:
            handler = &TKEngine::handleChoosingCandidate_;
            break;
        }
    }

    if (handler)
        return (this->*handler)(keyCode);
    return false;
}

EngineState TKEngine::getState() const { return engineState_; }

std::string TKEngine::getBuffer() const {
    return normalize(keyBuffer_, norm_nfc);
}

void TKEngine::popBack_() {
    if (keyBuffer_.empty()) {
        return;
    }

    keyBuffer_.pop_back();

    while (!utf8::is_valid(keyBuffer_.begin(), keyBuffer_.end())) {
        keyBuffer_.pop_back();
    }
}

int TKEngine::getDisplayBufferLength_() {
    int len = 0;

    for (int i = 0; i < displayBuffer_.segmentCount; i++) {
        if (i == displayBuffer_.selectedSegment &&
            engineState_ == EngineState::BufferByLetter) {
            len += displayBuffer_.segments[i].inputTextLength;
        } else {
            len += displayBuffer_.segments[i].displayTextLength;
        }
    }

    return len;
}

// Pro Mode key handler methods

bool TKEngine::onKeyDownPro_(KeyCode keyCode) {
    char c = (char)keyCode;

    if (keyBuffer_.empty()) {
        keyBuffer_.push_back(c);
        return true;
    }

    if (TONE_KEY_MAP.find(c) != TONE_KEY_MAP.end()) {
        string tone = _getToneOfKey(c);
        keyBuffer_ = normalize(placeToneOnSyllable(keyBuffer_, tone), norm_nfc);
    } else if (keyBuffer_.back() == 'o' &&
               c == SPEC_KEY_MAP.at(Special::OUCombo)) {
        keyBuffer_ += u8"\u0358";
    } else if (keyBuffer_.back() == 'n' &&
               c == SPEC_KEY_MAP.at(Special::NasalCombo)) {
        replace_last(keyBuffer_, u8"n", u8"\u207f");
    } else if (c == '\b') {
        popBack_();
    } else {
        keyBuffer_.push_back(c);
    }

    return true;
}

// Normal Mode key handler methods

bool TKEngine::handleKeyOnReady_(KeyCode keyCode) { return false; }

bool TKEngine::handleEditing_(KeyCode keyCode) {
    switch (keyCode) {
    case KeyCode::TK_TAB:
    case KeyCode::TK_DOWN:
        // switch to ByCandidates
        return true;
    case KeyCode::TK_SPACE:
        // switch to BySegment
        return true;
    default:
        // check print
        if (isprint((char)keyCode)) {
            // raw buffer + char
            // calculate new display buffer & cursor
        }
    }

    return false;
}

bool TKEngine::handleChoosingCandidate_(KeyCode keyCode) {
    switch (keyCode) {}
    return false;
}

bool TKEngine::handleNavByLetter_(KeyCode keyCode) {
    switch (keyCode) {}
    return false;
}

bool TKEngine::handleNavBySegment_(KeyCode keyCode) {
    switch (keyCode) {
    case KeyCode::TK_RIGHT:
        if (displayBuffer_.selectedSegment + 1 < displayBuffer_.segmentCount) {
            displayBuffer_.selectedSegment++;
        }
        return true;
    case KeyCode::TK_LEFT:
        if (displayBuffer_.selectedSegment > 0) {
            displayBuffer_.selectedSegment--;
        } else {
            setEngineState_(EngineState::BufferByLetter, keyCode);
        }
        return true;
    default:
        return false;
    }
}

void TKEngine::setEngineState_(EngineState nextEngineState,
                                   KeyCode keyCode) {
    EngineState prev = engineState_;
    engineState_ = nextEngineState;
    onChangeEngineState_(prev, nextEngineState, keyCode);
}

void TKEngine::onChangeEngineState_(EngineState prev, EngineState next,
                                        KeyCode keyCode) {
    switch (prev) {
    case EngineState::BufferBySegment:
        switch (next) {
        case EngineState::BufferByLetter:
            bySegmentToByLetter_(keyCode);
            break;
        }
        break;
    }
}

void TKEngine::bySegmentToByLetter_(KeyCode keyCode) {
    handleNavByLetter_(keyCode);
}

} // namespace TaiKey

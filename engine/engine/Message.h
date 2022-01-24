#pragma once

#include <string>
#include <vector>

/*
namespace khiin::messages {

//+---------------------------------------------------------------------------
//
// Request from App ===> Engine
//
//----------------------------------------------------------------------------

// Alphanumeric & hyphen use ASCII
// -  : 0x2D
// 0-9: 0x30-0x39
// A-Z: 0x41-0x5A
// a-z: 0x61-0x7A
using KeyCode = char;
using CandidateId = int;
using CursorPosition = int;

enum class SpecialKey {
    None,
    Space,
    Enter,
    Esc,
    Backspace,
    Tab,
    Left,
    Up,
    Right,
    Down,
    Pgup,
    Pgdn,
    Home,
    End,
    Del,
};

enum class ModifierKey {
    Ctrl,
    Alt,
    Shift,
};

enum class Command {
    None,
    Revert,
    Commit,
    SelectCandidate,
    FocusCandidate,
    SwitchInputMode,
    Reset,
    MoveCursor,
    Disable,
    Enable,
    SendKey,
    TestSendKey,
    SetConfig,
};

enum class CursorDirection {
    L,
    R,
};

struct KeyEvent {
    KeyCode key_code = 0;
    SpecialKey special_key = SpecialKey::None;
    std::vector<ModifierKey> modifier_keys = {};
};

struct Request {
    Command command = Command::None;
    KeyEvent key_event;
    CandidateId id = 0;
    //CursorPosition cursor_position;
};

//+---------------------------------------------------------------------------
//
// Response from Engine ===> App
//
//----------------------------------------------------------------------------

enum class BufferStatus {
    Composing,
    Converted,
    Focused,
};

struct Segment {
    std::string value;
    BufferStatus status;
};

struct Composition {
    CursorPosition cursor_position = 0;
    std::vector<Segment> text_segments;
};

enum class CandidateCategory {
    Normal,
    Kana,
    Rare,
};

struct Candidate {
    CandidateId id = 0;
    std::string value;
    std::string key;
    std::string annotation;
    CandidateCategory category;
};

struct Response {
    Composition composition;
    std::vector<Candidate> candidate_list;
    bool consumed;
};

} // namespace khiin::messages
*/
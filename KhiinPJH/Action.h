#pragma once

#include "pch.h"

namespace khiin::win32 {

enum class Message {
    Noop,
    StartComposition,
    UpdateComposition,
    CancelComposition,
    CommitText,
    ShowCandidates,
    HideCandidates,
    FocusCandidate,
};

struct Action {
    Message compMsg = Message::Noop;
    std::string text;
    Message candMsg = Message::Noop;
    std::vector<std::string> candidates;
};

} // namespace khiin::win32
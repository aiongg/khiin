#pragma once

#include "pch.h"

namespace Khiin {

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

} // namespace Khiin
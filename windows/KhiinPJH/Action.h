#pragma once

#include "pch.h"

namespace khiin::win32 {

enum class Message {
    Noop,
    Compose,
    CancelComposition,
    Commit,
    ShowCandidates,
    HideCandidates,
    FocusCandidate,
};

struct Action {
    bool consumed = false;
    Message compose_message = Message::Noop;
    std::string buffer_text;
    Message candidate_message = Message::Noop;
    std::vector<std::string> *candidate_list;
};

} // namespace khiin::win32

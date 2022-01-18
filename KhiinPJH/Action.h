#pragma once

#include "pch.h"

namespace Khiin {

enum class Message { Noop, StartComposition, UpdateComposition, CancelComposition, CommitText };

struct Action {
    Message msg = Message::Noop;
    std::string text;
    std::vector<std::string> candidates;
};

} // namespace Khiin
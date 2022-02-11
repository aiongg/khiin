#pragma once

#include "common.h"
#include "messages.h"

namespace khiin::engine {

enum class InputMode {
    Continuous,
    SingleToken,
    Manual,
};

class Engine;

// - Handles / routes commands from Engine
// - Builds the Preedit and CandidateList
// - Maintains the current caret position
class BufferMgr {
  public:
    static BufferMgr *Create(Engine *engine);
    virtual void Clear() = 0;
    virtual bool IsEmpty() = 0;
    virtual void Insert(char ch) = 0;
    virtual void MoveCaret(CursorDirection direction) = 0;
    virtual void Erase(CursorDirection direction) = 0;
    virtual void MoveFocus(CursorDirection direction) = 0;
    virtual void FocusCandidate(int index) = 0;
    virtual void SetInputMode(InputMode new_mode) = 0;
    virtual void BuildPreedit(messages::Preedit *preedit) = 0;
    virtual void GetCandidates(messages::CandidateList *candidate_list) = 0;
    virtual void SelectNextCandidate() = 0;
    virtual messages::EditState edit_state() = 0;
};

} // namespace khiin::engine

#pragma once

#include <string>
#include <vector>

#include "CandidateFinder.h"
#include "Config.h"
//#include "Splitter.h"
//#include "SynchronizedBuffer.h"
//#include "Trie.h"
#include "common.h"
//#include "errors.h"
#include "messages.h"

namespace khiin::engine {

struct CandidateDisplay {
    std::string text;
    int color = 0;
    std::string hint;
};

class BufferManager {
  public:
    static BufferManager *Create(CandidateFinder *candidate_finder);
    virtual void Clear() = 0;
    virtual bool IsEmpty() = 0;
    virtual void Insert(char ch) = 0;
    virtual void MoveCaret(CursorDirection dir) = 0;
    virtual void MoveFocus(CursorDirection dir) = 0;
    virtual void BuildPreedit(messages::Preedit* preedit) = 0;
    virtual void FocusCandidate(size_t index) = 0;
    virtual void Erase(CursorDirection dir) = 0;

    virtual std::vector<CandidateDisplay> getCandidates() = 0;
    virtual std::string getDisplayBuffer() = 0;
    virtual size_t getCursor() = 0;
    virtual void selectPrimaryCandidate() = 0;
    virtual void selectCandidate(size_t index) = 0;
    virtual void setToneKeys(ToneKeys toneKeys) = 0;
};

} // namespace khiin::engine

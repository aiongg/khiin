#include "BufferManager.h"

#include <variant>

namespace khiin::engine {

namespace {

using Token2 = int;

class BufferImpl2 : public BufferManager {
  public:
    using Element = std::variant<std::string, Token2>;

    BufferImpl2(CandidateFinder *candidate_finder) : candidate_finder(candidate_finder){};

    // Inherited via BufferManager
    virtual void Clear() override {}

    virtual bool IsEmpty() override {
        return false;
    }

    virtual void Insert(char ch) override {}

    virtual void MoveCaret(CursorDirection dir) override {}

    virtual void MoveFocus(CursorDirection dir) override {}

    virtual void BuildPreedit(messages::Preedit *preedit) override {}

    virtual void FocusCandidate(size_t index) override {}

    virtual void Erase(CursorDirection dir) override {}

    virtual size_t caret_position() override {
        return size_t();
    }

    // Below methods are in testing only

    virtual std::vector<CandidateDisplay> getCandidates() override {
        return std::vector<CandidateDisplay>();
    }

    virtual std::string getDisplayBuffer() override {
        return std::string();
    }

    virtual void selectPrimaryCandidate() override {}

    virtual void selectCandidate(size_t index) override {}

    virtual void setToneKeys(ToneKeys toneKeys) override {}

  private:
    CandidateFinder *candidate_finder = nullptr;
};

} // namespace

BufferManager *BufferManager::Create2(CandidateFinder *candidate_finder) {
    return new BufferImpl2(candidate_finder);
}

} // namespace khiin::engine

#include "BufferManager.h"

#include <regex>
#include <unordered_map>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <c9/zip.h>
#include <utf8cpp/utf8.h>

#include "Lomaji.h"
#include "SynchronizedBuffer.h"
#include "Trie.h"
#include "common.h"

namespace khiin::engine {
namespace {

using namespace messages;

static const std::regex kToneableLetters(u8"[aeioumn]");

auto isOnlyHyphens(std::string s) {
    for (const auto &c : s) {
        if (c != '-') {
            return false;
        }
    }

    return true;
}

auto candidateDisplayOf(Candidate candidate) -> CandidateDisplay {
    auto ret = CandidateDisplay();

    auto tokens = std::vector<std::string_view>();
    std::transform(candidate.cbegin(), candidate.cend(), std::back_inserter(tokens), [](const auto &chunk) {
        if (chunk.token.empty()) {
            return std::string_view(chunk.raw);
        }
        return std::string_view(chunk.token.output);
    });

    auto spaces = tokenSpacer(tokens);

    for (auto &&[spaced, token] : c9::zip(spaces, tokens)) {
        ret.text.append(token);
        if (spaced) {
            ret.text.push_back(' ');
        }
    }

    return ret;
}

void AddPreeditSegment(Preedit *preedit, const Segment &buf_segment) {
    auto pre_segment = preedit->add_segments();
    pre_segment->set_value(buf_segment.display_value);
    if (buf_segment.converted) {
        pre_segment->set_status(SegmentStatus::CONVERTED);
    } else if (buf_segment.focused) {
        pre_segment->set_status(SegmentStatus::FOCUSED);
    }
    if (buf_segment.spaced) {
        auto empty_segment = preedit->add_segments();
        empty_segment->set_value(" ");
    }
}

//+---------------------------------------------------------------------------
//
// BufferMgrImpl
//
//----------------------------------------------------------------------------
class BufferMgrImpl : public BufferManager {
  public:
    BufferMgrImpl(CandidateFinder *cf) : candidate_finder(cf) {}

    virtual void Clear() override {
        buffer.Reset();
        candidates.clear();
    }

    virtual bool IsEmpty() override {
        return buffer.empty();
    }

    // Inserts a character at the current cursor location
    virtual void Insert(char ch) override {
        /*
         * ch must match: [A-Za-z0-9-]
         *
         * Cases:
         *   1. Normal / Pro input mode
         *   2. Numeric / Telex tones
         *   3. Fuzzy / Exact tones
         *   4. Lazy / Quick commit
         */
        switch (inputMode_) {
        case InputMode::Normal:
            insertNormal(ch);
        }
    }

    virtual void Erase(CursorDirection dir) override {
        buffer.erase(dir);

        if (buffer.empty()) {
            Clear();
        } else {
            findPrimaryCandidate(buffer.caret(), buffer.end());
        }
    }

    virtual void MoveCaret(CursorDirection dir) override {
        buffer.MoveCaret(dir);
    }

    virtual void MoveFocus(CursorDirection dir) override {
        return;
    }

    virtual void BuildPreedit(Preedit *preedit) override {
        auto it = buffer.cbegin();
        auto end = buffer.cend();

        auto edit_begin = buffer.edit_cbegin();
        auto edit_end = buffer.edit_cend();

        while (it != edit_begin && it != end) {
            AddPreeditSegment(preedit, it[0]);
            ++it;
        }

        if (it == edit_begin && it != end) {
            auto value = std::string();
            while (it != edit_end && it != end) {
                value += it[0].display_value;
                if (it[0].spaced) {
                    value.push_back(' ');
                }
                ++it;
            }
            auto segment = preedit->add_segments();
            segment->set_value(value);
            segment->set_status(SegmentStatus::COMPOSING);
        }

        while (it != end) {
            AddPreeditSegment(preedit, it[0]);
            ++it;
        }

        preedit->set_cursor_position(buffer.displayCursorOffset());
    }

    virtual void FocusCandidate(size_t index) override {
        if (index >= candidates.size()) {
            throw std::invalid_argument("Candidate index out of range");
        }

        // TODO
    }

    virtual std::string getDisplayBuffer() override {
        return buffer.displayText();
    }

    virtual size_t getCursor() override {
        return buffer.displayCursorOffset();
    }

    virtual std::vector<CandidateDisplay> getCandidates() override {
        auto ret = std::vector<CandidateDisplay>();

        for (auto &c : candidates) {
            ret.push_back(candidateDisplayOf(c));
        }

        return ret;
    }

    virtual void setToneKeys(ToneKeys toneKeys) override {
        toneKeys_ = toneKeys;
    }

    virtual void selectPrimaryCandidate() override {
        if (!hasPrimaryCandidate) {
            return;
        }

        auto seg = buffer.edit_begin();
        auto cand = candidates[0].begin();

        for (auto i = 0; i < candidates[0].size(); ++i) {
            if (!cand->token.empty()) {
                seg->display_value = cand->token.output;
            }

            seg->converted = true;

            ++seg;
            ++cand;
        }

        buffer.updateSegmentSpacing();
        buffer.moveCursorToEnd();
    }

    virtual void selectCandidate(size_t index) override {
        if (index == 0 && hasPrimaryCandidate) {
            return selectPrimaryCandidate();
        }

        auto &selected = candidates[index];
    }

    //+---------------------------------------------------------------------------
    //
    // BufferMgrImpl private methods
    //
    //----------------------------------------------------------------------------
  private:
    auto isCursorAtEnd() -> bool {
        return buffer.isCursorAtEnd();
    }

    auto findCandidates(Segments::iterator begin) -> void {
        auto lgram = lgramOf(begin);
        auto searchStr = buffer.rawText(begin, buffer.edit_end());
        auto fuzzy = toneMode == ToneMode::Fuzzy;

        auto found = candidate_finder->findCandidates(searchStr, lgram, fuzzy);

        if (found.empty()) {
            return;
        }

        if (hasPrimaryCandidate) {
            auto &pc = candidates[0];
            found.erase(std::remove_if(found.begin(), found.end(),
                                       [&](const Candidate cand) {
                                           return cand[0].token == pc[0].token;
                                       }),
                        found.end());
        }

        for (auto &alt : found) {
            candidates.emplace_back(Candidate{std::move(alt)});
        }
    }

    auto findPrimaryCandidate(Segments::iterator begin, Segments::iterator last) -> void {
        auto offset = std::distance(buffer.begin(), begin);
        auto lgram = lgramOf(begin);
        auto searchStr = buffer.rawText(begin, last);
        auto fuzzy = toneMode == ToneMode::Fuzzy;

        auto primary = candidate_finder->findPrimaryCandidate(searchStr, lgram, fuzzy);

        if (primary.empty()) {
            return;
        }

        buffer.segmentByCandidate(begin, last, primary);
        // buffer_.setCandidates(first, std::move(candidates_));

        if (candidates.size() == 0) {
            candidates.push_back(std::move(primary));
            hasPrimaryCandidate = true;
            return;
        }

        if (candidates.size() > 0) {
            candidates.erase(candidates.begin() + 1, candidates.end());
            auto &lastprimary = candidates[0];
            lastprimary.erase(lastprimary.begin() + offset, lastprimary.end());
        }

        candidates[0].insert(candidates[0].end(), std::make_move_iterator(primary.begin()),
                             std::make_move_iterator(primary.end()));

        hasPrimaryCandidate = true;
    }

    auto insertNormal(char ch) -> RetVal {
        auto begin = buffer.begin();
        auto curs = buffer.caret();

        if (buffer.isCursorAtEnd()) {
            if (buffer.segmentCount() > 4) {
                begin = curs - 4;
            } else {
                begin = buffer.edit_begin();
            }
        } else {
            begin = curs;
        }

        auto offset = std::distance(buffer.begin(), begin);
        buffer.insert(ch);
        findPrimaryCandidate(begin, buffer.edit_end());
        if (offset == 0) {
            findCandidates(buffer.edit_begin());
        }

        return RetVal::TODO;
    }

    auto insertTelex_(char ch) -> RetVal {
        return RetVal::Error;
    }

    auto lgramOf(Segments::iterator segment) -> std::string {
        return segment == buffer.begin() ? std::string() : segment[-1].display_value;
    }

    SynchronizedBuffer buffer;
    CandidateFinder *candidate_finder = nullptr;
    char lastKey_ = '\0';
    CommitMode commitMode_ = CommitMode::Lazy;
    InputMode inputMode_ = InputMode::Normal;
    ToneKeys toneKeys_ = ToneKeys::Numeric;
    ToneMode toneMode = ToneMode::Fuzzy;
    Candidates candidates;
    bool hasPrimaryCandidate = false;
};

} // namespace

BufferManager *BufferManager::Create(CandidateFinder *candidate_finder) {
    return new BufferMgrImpl(candidate_finder);
}

} // namespace khiin::engine

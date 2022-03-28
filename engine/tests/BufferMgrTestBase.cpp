#include "BufferMgrTestBase.h"

namespace khiin::engine {
using namespace proto;
using ::testing::Contains;

void BufferMgrTestBase::SetInputMode(proto::InputMode mode) {
    auto conf = AppConfig();
    conf.set_input_mode(mode);
    TestEnv::engine()->config()->UpdateAppConfig(conf);
}

void BufferMgrTestBase::input(std::string str) {
    for (auto c : str) {
        bufmgr->Insert(c);
    }
}

void BufferMgrTestBase::curs_left(int n) {
    for (auto i = 0; i < n; i++) {
        bufmgr->HandleLeftRight(CursorDirection::L);
    }
}
void BufferMgrTestBase::curs_right(int n) {
    for (auto i = 0; i < n; i++) {
        bufmgr->HandleLeftRight(CursorDirection::R);
    }
}

void BufferMgrTestBase::curs_down(int n) {
    for (auto i = 0; i < n; ++i) {
        bufmgr->FocusNextCandidate();
    }
}

void BufferMgrTestBase::curs_up(int n) {
    for (auto i = 0; i < n; ++i) {
        bufmgr->FocusPrevCandidate();
    }
}

void BufferMgrTestBase::key_bksp(int n) {
    for (auto i = 0; i < n; i++) {
        bufmgr->Erase(CursorDirection::L);
    }
}

void BufferMgrTestBase::key_del(int n) {
    for (auto i = 0; i < n; i++) {
        bufmgr->Erase(CursorDirection::R);
    }
}

void BufferMgrTestBase::spacebar(int n) {
    for (auto i = 0; i < n; ++i) {
        bufmgr->HandleSelectOrFocus();
    }
}

void BufferMgrTestBase::enter() {
    bufmgr->HandleSelectOrCommit();
}

Preedit *BufferMgrTestBase::get_preedit() {
    auto preedit = new Preedit();
    bufmgr->BuildPreedit(preedit);
    return preedit;
}

Segments BufferMgrTestBase::get_segments() {
    auto preedit = get_preedit();
    return preedit->segments();
}

CandidateList *BufferMgrTestBase::get_candidates() {
    auto candlist = new CandidateList();
    bufmgr->GetCandidates(candlist);
    return candlist;
}

std::string BufferMgrTestBase::display() {
    auto preedit = new Preedit();
    bufmgr->BuildPreedit(preedit);
    auto ret = std::string();
    for (auto &segment : preedit->segments()) {
        ret += segment.value();
    }
    return ret;
}

size_t BufferMgrTestBase::caret() {
    auto preedit = new Preedit();
    bufmgr->BuildPreedit(preedit);
    return preedit->caret();
}

std::vector<std::string> BufferMgrTestBase::get_cand_strings() {
    auto cands = get_candidates()->candidates();
    auto ret = std::vector<std::string>();
    for (auto c : cands) {
        ret.push_back(c.value());
    }
    return ret;
}

std::string BufferMgrTestBase::CandidateAt(int candidate_index) {
    auto cands = get_cand_strings();
    return cands[candidate_index];
}

int BufferMgrTestBase::CandidateIndexOf(std::string candidate) {
    auto cands = get_cand_strings();
    for (size_t i = 0; i < cands.size(); ++i) {
        if (cands[i] == candidate) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

// Expectations

void BufferMgrTestBase::ExpectDisplay(std::string value) {
    EXPECT_EQ(display(), value);
}

void BufferMgrTestBase::ExpectCaret(int caret_index) {
    EXPECT_EQ(caret(), caret_index);
}

void BufferMgrTestBase::ExpectBuffer(std::string value, int caret_index) {
    EXPECT_EQ(display(), value);
    EXPECT_EQ(caret(), caret_index);
}

void BufferMgrTestBase::ExpectSegment(int segment_size, int segment_index, SegmentStatus segment_status,
                                      std::string segment_value, int caret_index) {
    auto segments = get_segments();
    EXPECT_EQ(caret(), caret_index);
    EXPECT_EQ(segments.size(), segment_size);
    EXPECT_EQ(segments.at(segment_index).value(), segment_value);
    EXPECT_EQ(segments.at(segment_index).status(), segment_status);
}

void BufferMgrTestBase::ExpectSegment(int segment_size, int segment_index, SegmentStatus segment_status) {
    auto segments = get_segments();
    EXPECT_EQ(segments.size(), segment_size);
    EXPECT_EQ(segments.at(segment_index).status(), segment_status);
}

void BufferMgrTestBase::ExpectEmpty() {
    EXPECT_TRUE(bufmgr->IsEmpty());
}

void BufferMgrTestBase::ExpectCandidatesHidden() {
    EXPECT_TRUE(get_candidates()->candidates().empty());
}

void BufferMgrTestBase::ExpectCandidateSize(int candidate_size) {
    EXPECT_EQ(get_candidates()->candidates_size(), candidate_size);
}

void BufferMgrTestBase::ExpectCandidate(std::string candidate) {
    auto cands = get_cand_strings();
    EXPECT_THAT(cands, Contains(candidate));
}

void BufferMgrTestBase::ExpectCandidate(std::string candidate, int candidate_index, bool focused) {
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.at(candidate_index), candidate);

    if (focused) {
        auto cand_list = get_candidates();
        EXPECT_EQ(cand_list->focused(), candidate_index);
    }
}

} // namespace khiin::engine
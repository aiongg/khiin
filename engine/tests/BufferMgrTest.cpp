#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "proto/proto.h"

#include "BufferMgr.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {
using namespace proto;
using ::testing::Contains;

bool OrEqual4(std::string test, std::string v1, std::string v2, std::string v3) {
    return test == v1 || test == v2 || test == v3;
}

bool OrEqual3(std::string test, std::string v1, std::string v2) {
    return test == v1 || test == v2;
}

struct BufferMgrTest : ::testing::Test {
  protected:
    BufferMgr *bufmgr = nullptr;
    Engine *engine = nullptr;

    void SetUp() override {
        bufmgr = TestEnv::engine()->buffer_mgr();
        bufmgr->Clear();
    }

    void typing(std::string str) {
        for (auto c : str) {
            bufmgr->Insert(c);
        }
    }

    void curs_left(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->HandleLeftRight(CursorDirection::L);
        }
    }
    void curs_right(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->HandleLeftRight(CursorDirection::R);
        }
    }

    void curs_down(int n) {
        for (auto i = 0; i < n; ++i) {
            bufmgr->FocusNextCandidate();
        }
    }

    void curs_up(int n) {
        for (auto i = 0; i < n; ++i) {
            bufmgr->FocusPrevCandidate();
        }
    }

    void key_bksp(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->Erase(CursorDirection::L);
        }
    }

    void key_del(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->Erase(CursorDirection::R);
        }
    }

    void spacebar(int n) {
        for (auto i = 0; i < n; ++i) {
            bufmgr->HandleSelectOrFocus();
        }
    }

    void enter() {
        bufmgr->HandleSelectOrCommit();
    }

    Preedit *get_preedit() {
        auto preedit = new Preedit();
        bufmgr->BuildPreedit(preedit);
        return preedit;
    }

    auto get_segments() {
        auto preedit = get_preedit();
        return preedit->segments();
    }

    CandidateList *get_candidates() {
        auto candlist = new CandidateList();
        bufmgr->GetCandidates(candlist);
        return candlist;
    }

    std::string display() {
        auto preedit = new Preedit();
        bufmgr->BuildPreedit(preedit);
        auto ret = std::string();
        for (auto &segment : preedit->segments()) {
            ret += segment.value();
        }
        return ret;
    }

    size_t caret() {
        auto preedit = new Preedit();
        bufmgr->BuildPreedit(preedit);
        return preedit->cursor_position();
    }

    std::vector<std::string> get_cand_strings() {
        auto cands = get_candidates()->candidates();
        auto ret = std::vector<std::string>();
        for (auto c : cands) {
            ret.push_back(c.value());
        }
        return ret;
    }

    std::string CandidateAt(int candidate_index) {
        auto cands = get_cand_strings();
        return cands[candidate_index];
    }

    int CandidateIndexOf(std::string candidate) {
        auto cands = get_cand_strings();
        for (auto i = 0; i < cands.size(); ++i) {
            if (cands[i] == candidate) {
                return i;
            }
        }

        return -1;
    }

    // Expectations

    void ExpectDisplay(std::string value) {
        EXPECT_EQ(display(), value);
    }

    void ExpectCaret(int caret_index) {
        EXPECT_EQ(caret(), caret_index);
    }

    void ExpectBuffer(std::string value, int caret_index) {
        EXPECT_EQ(display(), value);
        EXPECT_EQ(caret(), caret_index);
    }

    void ExpectSegment(int segment_size, int segment_index, SegmentStatus segment_status, std::string segment_value,
                       int caret_index) {
        auto segments = get_segments();
        EXPECT_EQ(caret(), caret_index);
        EXPECT_EQ(segments.size(), segment_size);
        EXPECT_EQ(segments.at(segment_index).value(), segment_value);
        EXPECT_EQ(segments.at(segment_index).status(), segment_status);
    }

    void ExpectSegment(int segment_size, int segment_index, SegmentStatus segment_status) {
        auto segments = get_segments();
        EXPECT_EQ(segments.size(), segment_size);
        EXPECT_EQ(segments.at(segment_index).status(), segment_status);
    }

    void ExpectEmpty() {
        EXPECT_TRUE(bufmgr->IsEmpty());
    }

    void ExpectCandidatesHidden() {
        EXPECT_TRUE(get_candidates()->candidates().empty());
    }

    void ExpectCandidateSize(int candidate_size) {
        EXPECT_EQ(get_candidates()->candidates_size(), candidate_size);
    }

    void ExpectCandidate(std::string candidate) {
        auto cands = get_cand_strings();
        EXPECT_THAT(cands, Contains(candidate));
    }

    void ExpectCandidate(std::string candidate, int candidate_index, bool focused) {
        auto cands = get_cand_strings();
        EXPECT_EQ(cands.at(candidate_index), candidate);

        if (focused) {
            auto cand_list = get_candidates();
            EXPECT_EQ(cand_list->focused(), candidate_index);
        }
    }
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, Reset) {
    typing("a");
    bufmgr->Clear();
    ExpectDisplay("");
    ExpectCaret(0);
}

//+---------------------------------------------------------------------------
//
// Insertions
//
//----------------------------------------------------------------------------

struct BufferInsertionTest : public BufferMgrTest {};

TEST_F(BufferInsertionTest, a) {
    typing("a");
    ExpectSegment(1, 0, SS_COMPOSING, "a", 1);
}

TEST_F(BufferInsertionTest, ta1) {
    typing("ta1");
    ExpectSegment(1, 0, SS_COMPOSING, "ta", 2);
}
TEST_F(BufferInsertionTest, taichi) {
    typing("t");
    ExpectSegment(1, 0, SS_COMPOSING, "t", 1);
    typing("a");
    ExpectSegment(1, 0, SS_COMPOSING, "ta", 2);
    typing("i");
    ExpectSegment(1, 0, SS_COMPOSING, "ta i", 4);
    typing("c");
    ExpectSegment(1, 0, SS_COMPOSING, "tai c", 5);
    typing("h");
    ExpectSegment(1, 0, SS_COMPOSING, "tai ch", 6);
    typing("i");
    ExpectSegment(1, 0, SS_COMPOSING, "tai chi", 7);
}

TEST_F(BufferInsertionTest, to7si7) {
    typing("t");
    ExpectSegment(1, 0, SS_COMPOSING, "t", 1);
    typing("o");
    ExpectSegment(1, 0, SS_COMPOSING, "to", 2);
    typing("7");
    ExpectSegment(1, 0, SS_COMPOSING, "tō", 2);
    typing("s");
    ExpectSegment(1, 0, SS_COMPOSING, "tō s", 4);
    typing("i");
    ExpectSegment(1, 0, SS_COMPOSING, "tō si", 5);
    typing("7");
    ExpectSegment(1, 0, SS_COMPOSING, "tō sī", 5);
}

TEST_F(BufferInsertionTest, tai7chi) {
    typing("tai7chi");
    ExpectSegment(1, 0, SS_COMPOSING, "tāi chi", 7);
}

TEST_F(BufferInsertionTest, ian9jin2) {
    typing("ian9");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn", 3);
    typing("j");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn j", 5);
    typing("i");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn ji", 6);
    typing("n");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn jin", 7);
    typing("2");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn jín", 7);
}

TEST_F(BufferInsertionTest, aan2) {
    typing("aan2");
    ExpectSegment(1, 0, SS_COMPOSING, "a án", 4);
}

TEST_F(BufferInsertionTest, len) {
    typing("len");
    ExpectSegment(1, 0, SS_COMPOSING, "len", 3);
}

TEST_F(BufferInsertionTest, mng7) {
    typing("m");
    ExpectSegment(1, 0, SS_COMPOSING, "m", 1);
    typing("n");
    ExpectSegment(1, 0, SS_COMPOSING, "mn", 2);
    typing("g");
    ExpectSegment(1, 0, SS_COMPOSING, "mng", 3);
    typing("7");
    ExpectSegment(1, 0, SS_COMPOSING, "mn\u0304g", 4);
}

TEST_F(BufferInsertionTest, Goa) {
    typing("Goa");
    ExpectSegment(1, 0, SS_COMPOSING, "Goa", 3);
}

//+---------------------------------------------------------------------------
//
// Caret movement
//
//----------------------------------------------------------------------------

struct BufferCaretTest : public BufferMgrTest {};

TEST_F(BufferCaretTest, Move_a) {
    typing("a");
    ExpectSegment(1, 0, SS_COMPOSING, "a", 1);

    curs_left(1);
    ExpectSegment(1, 0, SS_COMPOSING, "a", 0);

    curs_left(1);
    ExpectSegment(1, 0, SS_COMPOSING, "a", 0);

    curs_right(1);
    ExpectSegment(1, 0, SS_COMPOSING, "a", 1);

    curs_right(1);
    ExpectSegment(1, 0, SS_COMPOSING, "a", 1);
}

TEST_F(BufferCaretTest, Move_ah8) {
    typing("ah8");
    ExpectSegment(1, 0, SS_COMPOSING, "a\u030dh", 3);

    curs_left(1);
    ExpectCaret(2);

    curs_left(1);
    ExpectCaret(0);

    curs_right(1);
    ExpectCaret(2);

    curs_right(1);
    ExpectSegment(1, 0, SS_COMPOSING, "a\u030dh", 3);
}

TEST_F(BufferCaretTest, Move_sou2i2) {
    typing("sou2i2");
    ExpectSegment(1, 0, SS_COMPOSING, "só\u0358 í", 5);

    curs_left(1);
    ExpectCaret(4);

    curs_left(1);
    ExpectCaret(3);

    curs_left(1);
    ExpectCaret(1);

    curs_left(1);
    ExpectCaret(0);

    curs_right(1);
    ExpectCaret(1);

    curs_right(1);
    ExpectCaret(3);

    curs_right(1);
    ExpectCaret(4);

    curs_right(1);
    ExpectSegment(1, 0, SS_COMPOSING, "só\u0358 í", 5);
}

TEST_F(BufferCaretTest, MoveType_siongho) {
    typing("siongho");
    ExpectBuffer("siong ho", 8);
    curs_left(6);
    typing("h");
    ExpectBuffer("si hong ho", 4);
}

//+---------------------------------------------------------------------------
//
// Deletions
//
//----------------------------------------------------------------------------

struct BufferEraseTest : public BufferMgrTest {};

TEST_F(BufferEraseTest, Backspace_a) {
    typing("a");
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferEraseTest, Delete_a) {
    typing("a");
    curs_left(1);
    key_del(1);
    ExpectEmpty();
}

TEST_F(BufferEraseTest, Delete_taichi) {
    typing("taichi");
    ExpectBuffer("tai chi", 7);
    key_bksp(1);
    ExpectBuffer("tai ch", 6);
    key_bksp(1);
    ExpectBuffer("tai c", 5);
    key_bksp(1);
    ExpectBuffer("ta i", 4);
    key_bksp(1);
    ExpectBuffer("ta", 2);
    key_bksp(1);
    ExpectBuffer("t", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferEraseTest, Delete_taichi_vspace) {
    typing("taichi");
    ExpectBuffer("tai chi", 7);
    curs_left(3);
    ExpectBuffer("tai chi", 4);
    key_bksp(1);
    ExpectBuffer("tai chi", 3);
    key_del(1);
    ExpectBuffer("tai chi", 4);
}

TEST_F(BufferEraseTest, Delete_vspace_sibo) {
    typing("sibo");
    ExpectBuffer("si bo", 5);
    curs_left(2);
    ExpectBuffer("si bo", 3);
    key_bksp(1);
    ExpectBuffer("si bo", 2);
    key_del(1);
    ExpectBuffer("si bo", 3);
}

TEST_F(BufferEraseTest, Delete_random) {
    typing("bo5wdprsfnlji7");
    ExpectBuffer("bô wdprsfnl jī", 14);

    curs_left(2);
    key_bksp(9);
    ExpectBuffer("bô jī", 2);
}

TEST_F(BufferEraseTest, Delete_kah8a) {
    typing("kah8a");
    ExpectBuffer("ka\u030dh a", 6);
    curs_left(3);
    ExpectBuffer("ka\u030dh a", 3);
    key_bksp(1);
    ExpectBuffer("kha", 1);
}

//+---------------------------------------------------------------------------
//
// Khin
//
//----------------------------------------------------------------------------

struct BufferKhinTest : public BufferMgrTest {};

TEST_F(BufferKhinTest, Insert_khin_a) {
    typing("--a");
    ExpectBuffer("·a", 2);
}

TEST_F(BufferKhinTest, Insert_khin_ho2_a) {
    typing("ho2---a");
    ExpectBuffer("hó-·a", 5);
}

TEST_F(BufferKhinTest, Insert_autokhin) {
    typing("a--bobobobo");
    ExpectBuffer("a ·bo ·bo ·bo ·bo", 17);
}

TEST_F(BufferKhinTest, Insert_2khins) {
    typing("ho2--si7--bo5");
    ExpectBuffer("hó ·sī ·bô", 10);
}

TEST_F(BufferKhinTest, Delete_khins) {
    typing("--a");
    ExpectBuffer("·a", 2);
    key_bksp(1);
    ExpectBuffer("·", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferKhinTest, Delete_khin_an2) {
    typing("--an2");
    ExpectBuffer("·án", 3);
    key_bksp(1);
    ExpectBuffer("·á", 2);
    key_bksp(1);
    ExpectBuffer("·", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferKhinTest, Delete_khins2) {
    TestEnv::engine()->config()->mutable_dotted_khin()->set_value(false);
    typing("--a");
    ExpectBuffer("--a", 3);
    key_bksp(1);
    ExpectBuffer("--", 2);
    key_bksp(1);
    ExpectBuffer("-", 1);
    key_bksp(1);
    ExpectEmpty();
    TestEnv::engine()->config()->mutable_dotted_khin()->set_value(true);
}

TEST_F(BufferKhinTest, Delete_autokhin) {
    typing("--aa");
    ExpectBuffer("·a ·a", 5);
    key_bksp(1);
    ExpectBuffer("·a", 2);
}

TEST_F(BufferKhinTest, Delete_autkhin_hyphens) {
    TestEnv::engine()->config()->mutable_dotted_khin()->set_value(false);
    typing("--aa");
    ExpectBuffer("--a--a", 6);
    key_bksp(1);
    ExpectBuffer("--a", 3);
    TestEnv::engine()->config()->mutable_dotted_khin()->set_value(true);
}

//+---------------------------------------------------------------------------
//
// Candidates
//
//----------------------------------------------------------------------------

struct CandidatesTest : public BufferMgrTest {};

TEST_F(CandidatesTest, Candidates_taichi) {
    typing("taichi");
    ExpectCandidateSize(3);
    ExpectCandidate("事志");
    ExpectCandidate("代志");
    ExpectCandidate("tāi-chì");
}

TEST_F(CandidatesTest, Candidates_e) {
    typing("e");
    ExpectCandidateSize(8);
    ExpectCandidate("个");
    ExpectCandidate("兮");
    ExpectCandidate("鞋");
    ExpectCandidate("ê");
    ExpectCandidate("能");
    ExpectCandidate("會");
    ExpectCandidate("下");
    ExpectCandidate("ē");
}

TEST_F(CandidatesTest, Goa_goa) {
    typing("goa");
    ExpectCandidateSize(8);
    ExpectCandidate("我");
    ExpectCandidate("góa");
    bufmgr->Clear();
    typing("Goa");
    ExpectCandidateSize(8);
    ExpectCandidate("我");
    ExpectCandidate("Góa");
}

TEST_F(CandidatesTest, b) {
    typing("b");
    ExpectCandidateSize(1);
    ExpectCandidate("b");
}

TEST_F(CandidatesTest, bo) {
    typing("bo");
    ExpectCandidateSize(2);
    ExpectCandidate("無");
    ExpectCandidate("bô");
}

TEST_F(CandidatesTest, boe) {
    typing("boe");
    ExpectCandidateSize(10);
    curs_up(2);
    enter();
    curs_right(1);
    curs_up(1);
    curs_down(1);
    curs_up(1);
    ExpectCandidate("个");
    ExpectCandidate("ê");
}

TEST_F(CandidatesTest, boe2) {
    typing("boe");
    ExpectCandidateSize(10);
    curs_up(1);
    enter();
    // curs_down(1);
    // ExpectCandidate("个");
    // ExpectCandidate("ê");
}

//+---------------------------------------------------------------------------
//
// Conversions
//
//----------------------------------------------------------------------------

struct BufferConversionTest : public BufferMgrTest {};

TEST_F(BufferConversionTest, Convert_gina) {
    typing("gina");
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, "囝仔", 2);
}

TEST_F(BufferConversionTest, Convert_erase_ho2) {
    typing("ho2");
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, "好", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferConversionTest, Convert_erase_kamanne) {
    typing("kamanne");
    spacebar(1);
    ASSERT_PRED4(OrEqual4, display(), "咁按呢", "咁安呢", "敢按呢");
    key_bksp(1);
    ASSERT_PRED4(OrEqual4, display(), "咁按", "咁安", "敢按");
    key_bksp(1);
    ASSERT_PRED3(OrEqual3, display(), "咁", "敢");
    key_bksp(1);
    EXPECT_EQ(display(), u8"");
}

TEST_F(BufferConversionTest, Convert_erase_insert) {
    typing("siannebo");
    spacebar(1);
    curs_left(1);
    key_bksp(1);
    ExpectBuffer("省無", 1);
    typing("h");
    ExpectBuffer("省 h 無", 3);
    typing("o");
    ExpectBuffer("省 ho 無", 4);
}

TEST_F(BufferConversionTest, Convert_insert_middle) {
    typing("bunte");
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, "問題", 2);
    curs_left(1);
    ExpectSegment(1, 0, SS_FOCUSED, "問題", 1);
    typing("ho");
    ExpectBuffer("問 ho 題", 4);
    ExpectSegment(3, 0, SS_CONVERTED, "問", 4);
    ExpectSegment(3, 1, SS_COMPOSING, " ho ", 4);
    ExpectSegment(3, 2, SS_CONVERTED, "題", 4);
    spacebar(1);
    ExpectBuffer("問好題", 2);
    ExpectSegment(3, 1, SS_FOCUSED, "好", 2);
}

TEST_F(BufferConversionTest, Convert_insert_erase) {
    typing("bunte");
    spacebar(1);
    curs_left(1);
    typing("ho");
    key_bksp(2);
    ExpectSegment(1, 0, SS_COMPOSING, "問題", 1);
}

TEST_F(BufferConversionTest, Convert_e5) {
    typing("e5");
    ExpectCandidateSize(4);
    auto cand = CandidateAt(0);

    ExpectSegment(1, 0, SS_COMPOSING, "ê", 1);
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, cand, 1);
}

TEST_F(BufferConversionTest, Convert_ebe1) {
    typing("ebe");
    ExpectCandidateSize(9);
    ExpectSegment(1, 0, SS_COMPOSING, "e be", 4);
}

TEST_F(BufferConversionTest, Convert_ebe2) {
    typing("ebe");
    spacebar(1);
    ExpectSegment(2, 0, SS_FOCUSED, "个", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "未", 2);
}

TEST_F(BufferConversionTest, Convert_ebe3) {
    typing("ebe");
    spacebar(2);
    ExpectSegment(2, 0, SS_FOCUSED, "兮", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "未", 2);
}

TEST_F(BufferConversionTest, Convert_ebe4) {
    typing("ebe");
    spacebar(1);
    curs_right(1);
    spacebar(1);
    ExpectSegment(2, 0, SS_CONVERTED, "个", 2);
    ExpectSegment(2, 1, SS_FOCUSED, "袂", 2);
}

TEST_F(BufferConversionTest, Convert_boe1) {
    typing("boe");
    spacebar(1);
    ExpectCandidatesHidden();
    ExpectSegment(1, 0, SS_FOCUSED, "未", 1);
}

TEST_F(BufferConversionTest, Convert_boe2) {
    typing("boe");
    spacebar(8);
    ExpectSegment(2, 0, SS_FOCUSED, "無", 3);
    ExpectSegment(2, 1, SS_COMPOSING, " e", 3);
}

TEST_F(BufferConversionTest, Convert_boe3) {
    typing("boe");
    spacebar(9);
    ExpectSegment(2, 0, SS_FOCUSED, "bô", 4);
    ExpectSegment(2, 1, SS_COMPOSING, " e", 4);
}

TEST_F(BufferConversionTest, Convert_boe4) {
    typing("boe");
    spacebar(10);
    ExpectSegment(1, 0, SS_FOCUSED, "未", 1);
}

TEST_F(BufferConversionTest, Convert_eee_remove_e) {
    typing("eee");
    key_bksp(1);
    spacebar(1);
    ExpectSegment(2, 0, SS_FOCUSED, "个", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "个", 2);
}

//+---------------------------------------------------------------------------
//
// Buffer navigation
//
//----------------------------------------------------------------------------

struct BufferNavigationTest : public BufferMgrTest {};

TEST_F(BufferNavigationTest, Focus_element) {
    typing("kamanne");
    spacebar(1);
    ExpectSegment(2, 0, SS_FOCUSED);
    ExpectSegment(2, 1, SS_CONVERTED);

    curs_right(1);
    ExpectSegment(2, 0, SS_CONVERTED);
    ExpectSegment(2, 1, SS_FOCUSED);

    curs_left(1);
    ExpectSegment(2, 0, SS_FOCUSED);
    ExpectSegment(2, 1, SS_CONVERTED);
}

TEST_F(BufferNavigationTest, Focus_skips_vspace) {
    typing("boho");
    spacebar(2);
    enter();
    ExpectSegment(3, 0, SS_CONVERTED, "bô", 4);
    ExpectSegment(3, 1, SS_UNMARKED);
    ExpectSegment(3, 2, SS_FOCUSED);
}

TEST_F(BufferMgrTest, DISABLED_TmpTest) {
    typing("iniauchiaheanneoupoetemthangchhikimhiahanahesitihiasisinithia");
    typing("n");
}

//+---------------------------------------------------------------------------
//
// Candidate navigation
//
//----------------------------------------------------------------------------

struct CandidateNavigationTest : public BufferMgrTest {};

TEST_F(CandidateNavigationTest, Focus_boe) {
    typing("boe");
    curs_down(1);
    ExpectSegment(1, 0, SS_FOCUSED, "未", 1);
    ExpectCandidateSize(10);
}

TEST_F(CandidateNavigationTest, Focus_taichi) {
    typing("taichi");
    curs_down(1);
    ExpectSegment(1, 0, SS_FOCUSED, "事志", 2);
    ExpectCandidateSize(3);
}

TEST_F(CandidateNavigationTest, Focus_erase_e) {
    typing("e");
    curs_down(1);
    key_bksp(1);
    EXPECT_TRUE(bufmgr->IsEmpty());
}

TEST_F(CandidateNavigationTest, Focus_prev_e) {
    typing("ex");
    spacebar(1);
    curs_up(1);
    ExpectSegment(3, 0, SS_FOCUSED, "ē", 3);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 3);
    ExpectSegment(3, 2, SS_CONVERTED, "x", 3);
}

TEST_F(CandidateNavigationTest, Focus_xyz) {
    typing("boxyz");
    spacebar(1);
    ExpectSegment(3, 0, SS_FOCUSED, "無", 5);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 5);
    ExpectSegment(3, 2, SS_CONVERTED, "xyz", 5);

    curs_right(1);
    curs_down(1);
    ExpectCandidate("xyz", 0, true);
}

TEST_F(CandidateNavigationTest, Focus_si__bo) {
    typing("si--bo");
    curs_down(1);
}

//+---------------------------------------------------------------------------
//
// Candidate selection
//
//----------------------------------------------------------------------------

struct CandidateSelectionTest : public BufferMgrTest {};

TEST_F(CandidateSelectionTest, Select_e) {
    typing("e");
    curs_down(2);
    enter();
    ExpectSegment(1, 0, SS_FOCUSED, "兮", 1);
    ExpectCandidatesHidden();
}

TEST_F(CandidateSelectionTest, Select_ebe1) {
    typing("ebe");
    auto i = CandidateIndexOf("會");
    spacebar(i);
    enter();
    ExpectSegment(2, 0, SS_CONVERTED, "會", 2);
    ExpectSegment(2, 1, SS_FOCUSED, "未", 2);
}

TEST_F(CandidateSelectionTest, Select_ebe2) {
    typing("ebe");
    spacebar(2);
    ExpectSegment(2, 0, SS_FOCUSED, "兮", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "未", 2);
    enter();
    ExpectSegment(2, 0, SS_CONVERTED, "兮", 2);
    ExpectSegment(2, 1, SS_FOCUSED, "未", 2);
    ExpectCandidatesHidden();
    curs_right(1);
    spacebar(1);
    ExpectSegment(2, 0, SS_CONVERTED, "兮", 2);
    ExpectSegment(2, 1, SS_FOCUSED, "袂", 2);
}

TEST_F(CandidateSelectionTest, Select_ex) {
    typing("ex");
    spacebar(1);
    ExpectSegment(3, 0, SS_FOCUSED, "个", 3);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 3);
    ExpectSegment(3, 2, SS_CONVERTED, "x", 3);
    curs_down(1);
    ExpectSegment(3, 0, SS_FOCUSED, "兮", 3);
    enter();
    ExpectSegment(3, 0, SS_CONVERTED, "兮", 3);
    curs_right(2);
    curs_down(1);
}

TEST_F(CandidateSelectionTest, Select_goa21) {
    typing("goa21");
    key_bksp(1);
    spacebar(1);
}

//+---------------------------------------------------------------------------
//
// Punctuation candidates
//
//----------------------------------------------------------------------------

struct PunctuationTest : public BufferMgrTest {};

TEST_F(PunctuationTest, Input_period) {
    typing(".");
    ExpectCandidate("。");
}

} // namespace
} // namespace khiin::engine

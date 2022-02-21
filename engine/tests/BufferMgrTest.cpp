#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BufferMgr.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {
using namespace messages;
using ::testing::Contains;

bool OrEqual(std::string test, std::string v1, std::string v2) {
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

    Preedit *get_preedit() {
        auto preedit = Preedit::default_instance().New();
        bufmgr->BuildPreedit(preedit);
        return preedit;
    }

    auto get_segments() {
        auto preedit = get_preedit();
        return preedit->segments();
    }

    CandidateList *get_candidates() {
        auto candlist = CandidateList::default_instance().New();
        bufmgr->GetCandidates(candlist);
        return candlist;
    }

    std::string display() {
        auto preedit = messages::Preedit::default_instance().New();
        bufmgr->BuildPreedit(preedit);
        auto ret = std::string();
        for (auto &segment : preedit->segments()) {
            ret += segment.value();
        }
        return ret;
    }

    size_t caret() {
        auto preedit = messages::Preedit::default_instance().New();
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

    // Expectations

    void ExpectDisplay(std::string value) {
        EXPECT_EQ(display(), value);
    }

    void ExpectCaret(int caret_index) {
        EXPECT_EQ(caret(), caret_index);
    }

    void ExpectSegment(int segment_index, SegmentStatus segment_status, std::string segment_value) {
        auto segments = get_segments();
        EXPECT_EQ(segments.at(segment_index).value(), segment_value);
        EXPECT_EQ(segments.at(segment_index).status(), segment_status);
    }

    void ExpectSegmentSize(int segment_size) {
        auto segments = get_segments();
        EXPECT_EQ(segments.size(), segment_size);
    }

    void ExpectBuffer(int segment_size, int segment_index, SegmentStatus segment_status, std::string segment_value,
                      int caret_index) {
        ExpectCaret(caret_index);
        ExpectSegmentSize(segment_size);
        ExpectSegment(segment_index, segment_status, segment_value);
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
    ExpectBuffer(1, 0, COMPOSING, "a", 1);
}

TEST_F(BufferInsertionTest, taichi) {
    typing("t");
    ExpectBuffer(1, 0, COMPOSING, "t", 1);
    typing("a");
    ExpectBuffer(1, 0, COMPOSING, "ta", 2);
    typing("i");
    ExpectBuffer(1, 0, COMPOSING, "ta i", 4);
    typing("c");
    ExpectBuffer(1, 0, COMPOSING, "tai c", 5);
    typing("h");
    ExpectBuffer(1, 0, COMPOSING, "tai ch", 6);
    typing("i");
    ExpectBuffer(1, 0, COMPOSING, "tai chi", 7);
}

TEST_F(BufferInsertionTest, to7si7) {
    typing("t");
    ExpectBuffer(1, 0, COMPOSING, "t", 1);
    typing("o");
    ExpectBuffer(1, 0, COMPOSING, "to", 2);
    typing("7");
    ExpectBuffer(1, 0, COMPOSING, "tō", 2);
    typing("s");
    ExpectBuffer(1, 0, COMPOSING, "tō s", 4);
    typing("i");
    ExpectBuffer(1, 0, COMPOSING, "tō si", 5);
    typing("7");
    ExpectBuffer(1, 0, COMPOSING, "tō sī", 5);
}

TEST_F(BufferInsertionTest, tai7chi) {
    typing("tai7chi");
    ExpectBuffer(1, 0, COMPOSING, "tāi chi", 7);
}

TEST_F(BufferInsertionTest, ian9jin2) {
    typing("ian9");
    ExpectBuffer(1, 0, COMPOSING, "iăn", 3);
    typing("j");
    ExpectBuffer(1, 0, COMPOSING, "iăn j", 5);
    typing("i");
    ExpectBuffer(1, 0, COMPOSING, "iăn ji", 6);
    typing("n");
    ExpectBuffer(1, 0, COMPOSING, "iăn jin", 7);
    typing("2");
    ExpectBuffer(1, 0, COMPOSING, "iăn jín", 7);
}

TEST_F(BufferInsertionTest, aan2) {
    typing("aan2");
    ExpectBuffer(1, 0, COMPOSING, "a án", 4);
}

TEST_F(BufferInsertionTest, len) {
    typing("len");
    ExpectBuffer(1, 0, COMPOSING, "len", 3);
}

TEST_F(BufferInsertionTest, mng7) {
    typing("m");
    ExpectBuffer(1, 0, COMPOSING, "m", 1);
    typing("n");
    ExpectBuffer(1, 0, COMPOSING, "mn", 2);
    typing("g");
    ExpectBuffer(1, 0, COMPOSING, "mng", 3);
    typing("7");
    ExpectBuffer(1, 0, COMPOSING, "mn\u0304g", 4);
}

//+---------------------------------------------------------------------------
//
// Caret movement
//
//----------------------------------------------------------------------------

struct BufferCaretTest : public BufferMgrTest {};

TEST_F(BufferCaretTest, Move_a) {
    typing("a");
    ExpectBuffer(1, 0, COMPOSING, "a", 1);

    curs_left(1);
    ExpectBuffer(1, 0, COMPOSING, "a", 0);

    curs_left(1);
    ExpectBuffer(1, 0, COMPOSING, "a", 0);

    curs_right(1);
    ExpectBuffer(1, 0, COMPOSING, "a", 1);

    curs_right(1);
    ExpectBuffer(1, 0, COMPOSING, "a", 1);
}

TEST_F(BufferCaretTest, Move_ah8) {
    typing("ah8");
    ExpectBuffer(1, 0, COMPOSING, "a\u030dh", 3);

    curs_left(1);
    ExpectCaret(2);

    curs_left(1);
    ExpectCaret(0);

    curs_right(1);
    ExpectCaret(2);

    curs_right(1);
    ExpectBuffer(1, 0, COMPOSING, "a\u030dh", 3);
}

TEST_F(BufferCaretTest, Move_sou2i2) {
    typing("sou2i2");
    ExpectBuffer(1, 0, COMPOSING, "só\u0358 í", 5);

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
    ExpectBuffer(1, 0, COMPOSING, "só\u0358 í", 5);
}

TEST_F(BufferCaretTest, MoveType_siongho) {
    typing("siongho");
    EXPECT_EQ(display(), "siong ho");
    curs_left(6);
    typing("h");
    EXPECT_EQ(display(), "si ho ng ho");
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
    EXPECT_TRUE(bufmgr->IsEmpty());
}

TEST_F(BufferEraseTest, Delete_a) {
    typing("a");
    curs_left(1);
    key_del(1);
    EXPECT_TRUE(bufmgr->IsEmpty());
}

TEST_F(BufferEraseTest, Delete_taichi) {
    typing("taichi");
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 7);
    key_bksp(1);
    EXPECT_EQ(display(), "tai ch");
    EXPECT_EQ(caret(), 6);
    key_bksp(1);
    EXPECT_EQ(display(), "tai c");
    EXPECT_EQ(caret(), 5);
    key_bksp(1);
    EXPECT_EQ(display(), "ta i");
    EXPECT_EQ(caret(), 4);
    key_bksp(1);
    EXPECT_EQ(display(), "ta");
    EXPECT_EQ(caret(), 2);
    key_bksp(1);
    EXPECT_EQ(display(), "t");
    EXPECT_EQ(caret(), 1);
    key_bksp(1);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferEraseTest, Delete_taichi_vspace) {
    typing("taichi");
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 7);
    curs_left(3);
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 4);
    key_bksp(1);
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 3);
    key_del(1);
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 4);
}

TEST_F(BufferEraseTest, Delete_vspace_sibo) {
    typing("sibo");
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 5);
    curs_left(2);
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 3);
    key_bksp(1);
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 2);
    key_del(1);
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 3);
}

TEST_F(BufferEraseTest, Delete_random) {
    typing("bo5wdprsfnlji7");

    EXPECT_EQ(display(), "bô wdprsfnl jī");
    EXPECT_EQ(caret(), 14);

    curs_left(2);
    key_bksp(9);

    EXPECT_EQ(display(), "bô jī");
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferEraseTest, Delete_kah8a) {
    typing("kah8a");
    EXPECT_EQ(display(), u8"ka\u030dh a");
    EXPECT_EQ(caret(), 6);
    curs_left(3);
    EXPECT_EQ(caret(), 3);
    key_bksp(1);
    EXPECT_EQ(display(), "kha");
    EXPECT_EQ(caret(), 1);
}

//+---------------------------------------------------------------------------
//
// Khin
//
//----------------------------------------------------------------------------

struct BufferKhinTest : public BufferMgrTest {};

TEST_F(BufferKhinTest, Insert_khin_a) {
    typing("--a");
    EXPECT_EQ(display(), u8"\u00b7a");
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferKhinTest, Insert_khin_ho2_a) {
    typing("ho2---a");
    EXPECT_EQ(display(), u8"hó-\u00b7a");
    EXPECT_EQ(caret(), 5);
}

TEST_F(BufferKhinTest, Insert_autokhin) {
    typing("a--bobobobo");
    EXPECT_EQ(display(), u8"a ·bo ·bo ·bo ·bo");
    EXPECT_EQ(caret(), 17);
}

TEST_F(BufferKhinTest, Insert_2khins) {
    typing("ho2--si7--bo5");
    EXPECT_EQ(display(), u8"hó ·sī ·bô");
    EXPECT_EQ(caret(), 10);
}

//+---------------------------------------------------------------------------
//
// Candidates
//
//----------------------------------------------------------------------------

struct CandidatesTest : public BufferMgrTest {};

TEST_F(CandidatesTest, Candidates_taichi) {
    typing("taichi");
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.size(), 3);
    EXPECT_THAT(cands, Contains(u8"事志"));
    EXPECT_THAT(cands, Contains(u8"代志"));
    EXPECT_THAT(cands, Contains(u8"tāi-chì"));
}

TEST_F(CandidatesTest, Candidates_e) {
    typing("e");
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.size(), 8);
    EXPECT_THAT(cands, Contains(u8"个"));
    EXPECT_THAT(cands, Contains(u8"兮"));
    EXPECT_THAT(cands, Contains(u8"鞋"));
    EXPECT_THAT(cands, Contains(u8"ê"));
    EXPECT_THAT(cands, Contains(u8"能"));
    EXPECT_THAT(cands, Contains(u8"會"));
    EXPECT_THAT(cands, Contains(u8"下"));
    EXPECT_THAT(cands, Contains(u8"ē"));
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
    EXPECT_EQ(display(), u8"囝仔");
}

TEST_F(BufferConversionTest, Convert_erase_ho2) {
    typing("ho2");
    spacebar(1);
    key_bksp(1);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferConversionTest, Convert_erase_kamanne) {
    typing("kamanne");
    spacebar(1);
    ASSERT_PRED3(OrEqual, display(), "咁按呢", "敢按呢");
    key_bksp(1);
    ASSERT_PRED3(OrEqual, display(), "咁按", "敢按");
    key_bksp(1);
    ASSERT_PRED3(OrEqual, display(), "咁", "敢");
    key_bksp(1);
    EXPECT_EQ(display(), u8"");
}

TEST_F(BufferConversionTest, Convert_erase_insert) {
    typing("siannebo");
    spacebar(1);
    curs_left(1);
    key_bksp(1);
    EXPECT_EQ(display(), u8"是按無");
    EXPECT_EQ(caret(), 2);
    typing("h");
    EXPECT_EQ(display(), u8"是按 h 無");
    EXPECT_EQ(caret(), 4);
    typing("o");
    EXPECT_EQ(display(), u8"是按 ho 無");
    EXPECT_EQ(caret(), 5);
}

TEST_F(BufferConversionTest, Convert_insert_middle) {
    typing("anne");
    spacebar(1);
    curs_left(1);
    typing("h");
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 3);
    typing("o");
    EXPECT_EQ(display(), u8"按 ho 呢");
    spacebar(1);
    EXPECT_EQ(display(), u8"按好呢");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 3);
    EXPECT_EQ(preedit->segments().at(1).status(), SegmentStatus::FOCUSED);
}

TEST_F(BufferConversionTest, Convert_insert_erase) {
    typing("anne");
    spacebar(1);
    curs_left(1);
    typing("ho");
    key_bksp(2);
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 1);
    EXPECT_EQ(display(), u8"按呢");
}

TEST_F(BufferConversionTest, Convert_e5) {
    typing("e5");
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.size(), 4);

    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"ê");
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::COMPOSING);
    EXPECT_EQ(caret(), 1);
    spacebar(1);
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), cands[0]);
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferConversionTest, Convert_ebe1) {
    typing("ebe");
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.size(), 9);

    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments.at(0).value(), "e be");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::COMPOSING);
    EXPECT_EQ(caret(), 4);
}

TEST_F(BufferConversionTest, Convert_ebe2) {
    typing("ebe");
    spacebar(1);

    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments.at(0).value(), "个");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(segments.at(1).value(), "未");
    EXPECT_EQ(segments.at(1).status(), SegmentStatus::CONVERTED);
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferConversionTest, Convert_ebe3) {
    typing("ebe");
    spacebar(2);

    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments.at(0).value(), "兮");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(segments.at(1).value(), "未");
    EXPECT_EQ(segments.at(1).status(), SegmentStatus::CONVERTED);
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferConversionTest, Convert_ebe4) {
    typing("ebe");
    spacebar(1);
    curs_right(1);
    spacebar(1);

    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments.at(0).value(), "个");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::CONVERTED);
    EXPECT_EQ(segments.at(1).value(), "袂");
    EXPECT_EQ(segments.at(1).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferConversionTest, Convert_boe1) {
    typing("boe");
    spacebar(1);
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_TRUE(get_candidates()->candidates().empty());
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments.at(0).value(), "未");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferConversionTest, Convert_boe2) {
    typing("boe");
    spacebar(8);
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments.at(0).value(), "無");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(segments.at(1).value(), " e");
    EXPECT_EQ(segments.at(1).status(), SegmentStatus::COMPOSING);
    EXPECT_EQ(caret(), 3);
}

TEST_F(BufferConversionTest, Convert_boe3) {
    typing("boe");
    spacebar(9);
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments.at(0).value(), "bô");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(segments.at(1).value(), " e");
    EXPECT_EQ(segments.at(1).status(), SegmentStatus::COMPOSING);
    EXPECT_EQ(caret(), 4);
}

TEST_F(BufferConversionTest, Convert_boe4) {
    typing("boe");
    spacebar(10);
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments.at(0).value(), "未");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 1);
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

    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 2);
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(preedit->segments().at(1).status(), SegmentStatus::CONVERTED);

    curs_right(1);
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 2);
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::CONVERTED);
    EXPECT_EQ(preedit->segments().at(1).status(), SegmentStatus::FOCUSED);

    curs_left(1);
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 2);
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(preedit->segments().at(1).status(), SegmentStatus::CONVERTED);
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
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(get_candidates()->candidates_size(), 10);
    EXPECT_EQ(segments.size(), 1);
    ExpectSegment(0, SegmentStatus::FOCUSED, "未");
    EXPECT_EQ(caret(), 1);
}

TEST_F(CandidateNavigationTest, Focus_taichi) {
    typing("taichi");
    curs_down(1);
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(get_candidates()->candidates_size(), 3);
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments.at(0).value(), "事志");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 2);
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
    auto s = get_segments();
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s.at(0).value(), "ē");
    EXPECT_EQ(s.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(s.at(1).value(), " ");
    EXPECT_EQ(s.at(1).status(), SegmentStatus::CONVERTED);
    EXPECT_EQ(s.at(2).value(), "x");
    EXPECT_EQ(s.at(2).status(), SegmentStatus::CONVERTED);
}

TEST_F(CandidateNavigationTest, Focus_xyz) {
    typing("boxyz");
    spacebar(1);
    auto s = get_segments();
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s.at(0).value(), "無");
    EXPECT_EQ(s.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(s.at(1).value(), " ");
    EXPECT_EQ(s.at(1).status(), SegmentStatus::CONVERTED);
    EXPECT_EQ(s.at(2).value(), "xyz");
    EXPECT_EQ(s.at(2).status(), SegmentStatus::CONVERTED);
    curs_right(1);
    curs_down(1);

    auto cands = get_candidates();
    auto &c = cands->candidates();
    EXPECT_EQ(c.size(), 1);
    EXPECT_EQ(c.at(0).value(), "xyz");
    EXPECT_EQ(cands->focused(), 0);
}

} // namespace
} // namespace khiin::engine

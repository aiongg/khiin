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
    void SetUp() override {
        bufmgr = TestEnv::engine()->buffer_mgr();
        bufmgr->Clear();
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

    void typing(std::string str) {
        for (auto c : str) {
            bufmgr->Insert(c);
        }
    }

    void curs_left(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->MoveCaret(CursorDirection::L);
        }
    }
    void curs_right(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->MoveCaret(CursorDirection::R);
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

    CandidateList *get_candidates() {
        auto candlist = CandidateList::default_instance().New();
        bufmgr->GetCandidates(candlist);
        return candlist;
    }

    std::vector<std::string> get_cand_strings() {
        auto cands = get_candidates()->candidates();
        auto ret = std::vector<std::string>();
        for (auto c : cands) {
            ret.push_back(c.value());
        }
        return ret;
    }

    BufferMgr *bufmgr = nullptr;
    Engine *engine = nullptr;
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, Reset) {
    typing("a");
    bufmgr->Clear();
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

//+---------------------------------------------------------------------------
//
// Insertions
//
//----------------------------------------------------------------------------

struct BufferInsertionTest : public BufferMgrTest {};

TEST_F(BufferInsertionTest, a) {
    typing("a");
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
    EXPECT_EQ(preedit->cursor_position(), 1);
}

TEST_F(BufferInsertionTest, taichi) {
    typing("t");
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "t");
    EXPECT_EQ(preedit->cursor_position(), 1);

    typing("a");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "ta");
    EXPECT_EQ(preedit->cursor_position(), 2);

    typing("i");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "ta i");
    EXPECT_EQ(preedit->cursor_position(), 4);

    typing("c");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "tai c");
    EXPECT_EQ(preedit->cursor_position(), 5);

    typing("h");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "tai ch");
    EXPECT_EQ(preedit->cursor_position(), 6);

    typing("i");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "tai chi");
    EXPECT_EQ(preedit->cursor_position(), 7);
}

TEST_F(BufferInsertionTest, to7si7) {
    typing("t");
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "t");
    EXPECT_EQ(preedit->cursor_position(), 1);

    typing("o");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"to");
    EXPECT_EQ(preedit->cursor_position(), 2);

    typing("7");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō");
    EXPECT_EQ(preedit->cursor_position(), 2);

    typing("s");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō s");
    EXPECT_EQ(preedit->cursor_position(), 4);

    typing("i");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō si");
    EXPECT_EQ(preedit->cursor_position(), 5);

    typing("7");
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō sī");
    EXPECT_EQ(preedit->cursor_position(), 5);
}

TEST_F(BufferInsertionTest, tai7chi) {
    typing("tai7chi");
    EXPECT_EQ(display(), u8"tāi chi");
    EXPECT_EQ(caret(), 7);
}

TEST_F(BufferInsertionTest, ian9jin2) {
    typing("ian9");
    EXPECT_EQ(display(), u8"iăn");
    typing("j");
    EXPECT_EQ(display(), u8"iăn j");
    typing("i");
    EXPECT_EQ(display(), u8"iăn ji");
    typing("n");
    EXPECT_EQ(display(), u8"iăn jin");
    typing("2");
    EXPECT_EQ(display(), u8"iăn jín");
    EXPECT_EQ(caret(), 7);
}

TEST_F(BufferInsertionTest, aan2) {
    typing("aan2");
    EXPECT_EQ(display(), u8"a án");
}

TEST_F(BufferInsertionTest, len) {
    typing("len");
    EXPECT_EQ(display(), u8"len");
}

TEST_F(BufferInsertionTest, mng7) {
    typing("m");
    EXPECT_EQ(display(), "m");
    EXPECT_EQ(caret(), 1);
    typing("n");
    EXPECT_EQ(display(), "mn");
    EXPECT_EQ(caret(), 2);
    typing("g");
    EXPECT_EQ(display(), "mng");
    EXPECT_EQ(caret(), 3);
    typing("7");
    EXPECT_EQ(display(), u8"mn\u0304g");
    EXPECT_EQ(caret(), 4);
}

//+---------------------------------------------------------------------------
//
// Caret movement
//
//----------------------------------------------------------------------------

struct BufferCaretTest : public BufferMgrTest {};

TEST_F(BufferCaretTest, Move_a) {
    typing("a");
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 0);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 0);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 1);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferCaretTest, Move_ah8) {
    typing("ah8");
    EXPECT_EQ(caret(), 3);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 2);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 0);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 2);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 3);
}

TEST_F(BufferCaretTest, Move_sou2i2) {
    typing("sou2i2"); // só͘ í
    EXPECT_EQ(caret(), 5);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 4);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 3);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 1);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 0);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 1);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 3);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 4);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 5);
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
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferEraseTest, Delete_a) {
    typing("a");
    curs_left(1);
    bufmgr->Erase(CursorDirection::R);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferEraseTest, Delete_taichi) {
    typing("taichi");
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 7);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "tai ch");
    EXPECT_EQ(caret(), 6);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "tai c");
    EXPECT_EQ(caret(), 5);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "ta i");
    EXPECT_EQ(caret(), 4);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "ta");
    EXPECT_EQ(caret(), 2);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "t");
    EXPECT_EQ(caret(), 1);
    bufmgr->Erase(CursorDirection::L);
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

struct BufferCandidatesTest : public BufferMgrTest {};

TEST_F(BufferMgrTest, Candidates_taichi) {
    typing("taichi");
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.size(), 3);
    EXPECT_THAT(cands, Contains(u8"事志"));
    EXPECT_THAT(cands, Contains(u8"代志"));
    EXPECT_THAT(cands, Contains(u8"tāi-chì"));
}

TEST_F(BufferMgrTest, Candidates_e) {
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

TEST_F(BufferConversionTest, Focus_boe) {
    typing("boe");
    curs_down(1);
    auto preedit = get_preedit();
    auto segments = preedit->segments();
    EXPECT_EQ(get_candidates()->candidates_size(), 10);
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments.at(0).value(), "未");
    EXPECT_EQ(segments.at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferConversionTest, Focus_taichi) {
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

//+---------------------------------------------------------------------------
//
// Converted navigation
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

} // namespace
} // namespace khiin::engine

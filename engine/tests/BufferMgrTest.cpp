#include "BufferMgrBaseTest.h"

#include "data/Database.h"

namespace khiin::engine {
using namespace proto;

struct BufferMgrTest : ::testing::Test, BufferMgrTestBase {
  protected:
    void SetUp() override {
        bufmgr = engine()->buffer_mgr();
        bufmgr->Clear();
    }
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, Reset) {
    input("a");
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
    input("a");
    ExpectSegment(1, 0, SS_COMPOSING, "a", 1);
}

TEST_F(BufferInsertionTest, ta1) {
    input("ta1");
    ExpectSegment(1, 0, SS_COMPOSING, "ta", 2);
}
TEST_F(BufferInsertionTest, taichi) {
    input("t");
    ExpectSegment(1, 0, SS_COMPOSING, "t", 1);
    input("a");
    ExpectSegment(1, 0, SS_COMPOSING, "ta", 2);
    input("i");
    ExpectSegment(1, 0, SS_COMPOSING, "ta i", 4);
    input("c");
    ExpectSegment(1, 0, SS_COMPOSING, "tai c", 5);
    input("h");
    ExpectSegment(1, 0, SS_COMPOSING, "tai ch", 6);
    input("i");
    ExpectSegment(1, 0, SS_COMPOSING, "tai chi", 7);
}

TEST_F(BufferInsertionTest, to7si7) {
    input("t");
    ExpectSegment(1, 0, SS_COMPOSING, "t", 1);
    input("o");
    ExpectSegment(1, 0, SS_COMPOSING, "to", 2);
    input("7");
    ExpectSegment(1, 0, SS_COMPOSING, "tō", 2);
    input("s");
    ExpectSegment(1, 0, SS_COMPOSING, "tō s", 4);
    input("i");
    ExpectSegment(1, 0, SS_COMPOSING, "tō si", 5);
    input("7");
    ExpectSegment(1, 0, SS_COMPOSING, "tō sī", 5);
}

TEST_F(BufferInsertionTest, tai7chi) {
    input("tai7chi");
    ExpectSegment(1, 0, SS_COMPOSING, "tāi chi", 7);
}

TEST_F(BufferInsertionTest, ian9jin2) {
    input("ian9");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn", 3);
    input("j");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn j", 5);
    input("i");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn ji", 6);
    input("n");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn jin", 7);
    input("2");
    ExpectSegment(1, 0, SS_COMPOSING, "iăn jín", 7);
}

TEST_F(BufferInsertionTest, aan2) {
    input("aan2");
    ExpectSegment(1, 0, SS_COMPOSING, "a án", 4);
}

TEST_F(BufferInsertionTest, len) {
    input("len");
    ExpectSegment(1, 0, SS_COMPOSING, "len", 3);
}

TEST_F(BufferInsertionTest, mng7) {
    input("m");
    ExpectSegment(1, 0, SS_COMPOSING, "m", 1);
    input("n");
    ExpectSegment(1, 0, SS_COMPOSING, "mn", 2);
    input("g");
    ExpectSegment(1, 0, SS_COMPOSING, "mng", 3);
    input("7");
    ExpectSegment(1, 0, SS_COMPOSING, "mn\u0304g", 4);
}

TEST_F(BufferInsertionTest, Goa) {
    input("Goa");
    ExpectSegment(1, 0, SS_COMPOSING, "Goa", 3);
}

//+---------------------------------------------------------------------------
//
// Caret movement
//
//----------------------------------------------------------------------------

struct BufferCaretTest : public BufferMgrTest {};

TEST_F(BufferCaretTest, Move_a) {
    input("a");
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
    input("ah8");
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
    input("sou2i2");
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
    input("siongho");
    ExpectBuffer("siong ho", 8);
    curs_left(6);
    input("h");
    ExpectBuffer("si hong ho", 4);
}

//+---------------------------------------------------------------------------
//
// Deletions
//
//----------------------------------------------------------------------------

struct BufferEraseTest : public BufferMgrTest {};

TEST_F(BufferEraseTest, Backspace_a) {
    input("a");
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferEraseTest, Delete_a) {
    input("a");
    curs_left(1);
    key_del(1);
    ExpectEmpty();
}

TEST_F(BufferEraseTest, Delete_taichi) {
    input("taichi");
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
    input("taichi");
    ExpectBuffer("tai chi", 7);
    curs_left(3);
    ExpectBuffer("tai chi", 4);
    key_bksp(1);
    ExpectBuffer("tai chi", 3);
    key_del(1);
    ExpectBuffer("tai chi", 4);
}

TEST_F(BufferEraseTest, Delete_vspace_sibo) {
    input("sibo");
    ExpectBuffer("si bo", 5);
    curs_left(2);
    ExpectBuffer("si bo", 3);
    key_bksp(1);
    ExpectBuffer("si bo", 2);
    key_del(1);
    ExpectBuffer("si bo", 3);
}

TEST_F(BufferEraseTest, Delete_random) {
    input("bo5wdprsfnlji7");
    ExpectBuffer("bô wdprsfnl jī", 14);

    curs_left(2);
    key_bksp(9);
    ExpectBuffer("bô jī", 2);
}

TEST_F(BufferEraseTest, Delete_kah8a) {
    input("kah8a");
    ExpectBuffer("ka\u030dh a", 6);
    curs_left(3);
    ExpectBuffer("ka\u030dh a", 3);
    key_bksp(1);
    ExpectBuffer("kha", 1);
}

TEST_F(BufferEraseTest, Delete_hox) {
    input("hox");
    ExpectBuffer("ho x", 4);
    spacebar(1);
    key_bksp(1);
    ExpectSegment(1, 0, SS_FOCUSED, "好", 1);
}

TEST_F(BufferEraseTest, Delete_ho_x) {
    input("ho");
    spacebar(1);
    ExpectBuffer("好", 1);
    input("x");
    key_bksp(1);
    ExpectSegment(1, 0, SS_FOCUSED, "好", 1);
}


//+---------------------------------------------------------------------------
//
// Khin
//
//----------------------------------------------------------------------------

struct BufferKhinTest : public BufferMgrTest {};

TEST_F(BufferKhinTest, Insert_khin_a) {
    input("--a");
    ExpectBuffer("·a", 2);
}

TEST_F(BufferKhinTest, Insert_khin_ho2_a) {
    input("ho2---a");
    ExpectBuffer("hó-·a", 5);
}

TEST_F(BufferKhinTest, Insert_autokhin) {
    input("a--bobobobo");
    ExpectBuffer("a ·bo ·bo ·bo ·bo", 17);
}

TEST_F(BufferKhinTest, Insert_2khins) {
    input("ho2--si7--bo5");
    ExpectBuffer("hó ·sī ·bô", 10);
}

TEST_F(BufferKhinTest, Delete_khins) {
    input("--a");
    ExpectBuffer("·a", 2);
    key_bksp(1);
    ExpectBuffer("·", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferKhinTest, Delete_khin_an2) {
    input("--an2");
    ExpectBuffer("·án", 3);
    key_bksp(1);
    ExpectBuffer("·á", 2);
    key_bksp(1);
    ExpectBuffer("·", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferKhinTest, Delete_khins2) {
    engine()->config()->set_dotted_khin(false);
    input("--a");
    ExpectBuffer("--a", 3);
    key_bksp(1);
    ExpectBuffer("--", 2);
    key_bksp(1);
    ExpectBuffer("-", 1);
    key_bksp(1);
    ExpectEmpty();
    engine()->config()->set_dotted_khin(true);
}

TEST_F(BufferKhinTest, Delete_autokhin) {
    input("--aa");
    ExpectBuffer("·a ·a", 5);
    key_bksp(1);
    ExpectBuffer("·a", 2);
}

TEST_F(BufferKhinTest, Delete_autkhin_hyphens) {
    engine()->config()->set_dotted_khin(false);
    input("--aa");
    ExpectBuffer("--a--a", 6);
    key_bksp(1);
    ExpectBuffer("--a", 3);
    engine()->config()->set_dotted_khin(true);
}

TEST_F(BufferKhinTest, Autokhin_disabled) {
    engine()->config()->set_autokhin(false);
    input("--aa");
    ExpectBuffer("·a a", 4);
    key_bksp(1);
    ExpectBuffer("·a", 2);
    engine()->config()->set_autokhin(true);
}

//+---------------------------------------------------------------------------
//
// Candidates
//
//----------------------------------------------------------------------------

struct CandidatesTest : public BufferMgrTest {};

TEST_F(CandidatesTest, Candidates_taichi) {
    input("taichi");
    ExpectCandidateSize(3);
    ExpectCandidate("事志");
    ExpectCandidate("代志");
    ExpectCandidate("tāi-chì");
}

TEST_F(CandidatesTest, Candidates_e) {
    input("e");
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
    input("goa");
    ExpectCandidateSize(8);
    ExpectCandidate("我");
    ExpectCandidate("góa");
    bufmgr->Clear();
    input("Goa");
    ExpectCandidateSize(8);
    ExpectCandidate("我");
    ExpectCandidate("Góa");
}

TEST_F(CandidatesTest, b) {
    input("b");
    ExpectCandidateSize(1);
    ExpectCandidate("b");
}

TEST_F(CandidatesTest, bo) {
    input("bo");
    ExpectCandidateSize(2);
    ExpectCandidate("無");
    ExpectCandidate("bô");
}

TEST_F(CandidatesTest, boe) {
    input("boe");
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
    input("boe");
    ExpectCandidateSize(10);
    curs_up(1);
    enter();
    // curs_down(1);
    // ExpectCandidate("个");
    // ExpectCandidate("ê");
}

TEST_F(CandidatesTest, gu5) {
    input("gu5");
    ExpectCandidateSize(3);
    ExpectCandidate("牛");
    ExpectCandidate("gû");
    ExpectCandidate("Gû");
}

//+---------------------------------------------------------------------------
//
// Conversions
//
//----------------------------------------------------------------------------

struct BufferConversionTest : public BufferMgrTest {};

TEST_F(BufferConversionTest, Convert_gina) {
    input("gina");
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, "囝仔", 2);
}

TEST_F(BufferConversionTest, Convert_erase_ho2) {
    input("ho2");
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, "好", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BufferConversionTest, Convert_erase_kamanne) {
    input("kamanne");
    spacebar(1);
    ASSERT_PRED4(OrEqual(), display(), "咁按呢", "咁安呢", "敢按呢");
    key_bksp(1);
    ASSERT_PRED4(OrEqual(), display(), "咁按", "咁安", "敢按");
    key_bksp(1);
    ASSERT_PRED3(OrEqual(), display(), "咁", "敢");
    key_bksp(1);
    EXPECT_EQ(display(), u8"");
}

TEST_F(BufferConversionTest, Convert_erase_insert) {
    input("siannebo");
    spacebar(1);
    curs_left(1);
    key_bksp(1);
    ExpectBuffer("省無", 1);
    input("h");
    ExpectBuffer("省 h 無", 3);
    input("o");
    ExpectBuffer("省 ho 無", 4);
}

TEST_F(BufferConversionTest, Convert_insert_middle) {
    input("bunte");
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, "問題", 2);
    curs_left(1);
    ExpectSegment(1, 0, SS_FOCUSED, "問題", 1);
    input("ho");
    ExpectBuffer("問 ho 題", 4);
    ExpectSegment(3, 0, SS_CONVERTED, "問", 4);
    ExpectSegment(3, 1, SS_COMPOSING, " ho ", 4);
    ExpectSegment(3, 2, SS_CONVERTED, "題", 4);
    spacebar(1);
    ExpectBuffer("問好題", 3);
    ExpectSegment(3, 0, SS_CONVERTED, "問", 3);
    ExpectSegment(3, 1, SS_FOCUSED, "好", 3);
    ExpectSegment(3, 2, SS_CONVERTED, "題", 3);
}

TEST_F(BufferConversionTest, Convert_insert_erase) {
    input("bunte");
    spacebar(1);
    curs_left(1);
    input("ho");
    key_bksp(2);
    ExpectSegment(2, 0, SS_FOCUSED, "問", 1);
    ExpectSegment(2, 1, SS_CONVERTED, "題", 1);
}

TEST_F(BufferConversionTest, Convert_e5) {
    input("e5");
    ExpectCandidateSize(4);
    auto cand = CandidateAt(0);

    ExpectSegment(1, 0, SS_COMPOSING, "ê", 1);
    spacebar(1);
    ExpectSegment(1, 0, SS_FOCUSED, cand, 1);
}

TEST_F(BufferConversionTest, Convert_ebe1) {
    input("ebe");
    ExpectCandidateSize(9);
    ExpectSegment(1, 0, SS_COMPOSING, "e be", 4);
}

TEST_F(BufferConversionTest, Convert_ebe2) {
    input("ebe");
    spacebar(1);
    ExpectSegment(2, 0, SS_FOCUSED, "个", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "未", 2);
}

TEST_F(BufferConversionTest, Convert_ebe3) {
    input("ebe");
    spacebar(2);
    ExpectSegment(2, 0, SS_FOCUSED, "兮", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "未", 2);
}

TEST_F(BufferConversionTest, Convert_ebe4) {
    input("ebe");
    spacebar(1);
    curs_right(1);
    spacebar(1);
    ExpectSegment(2, 0, SS_CONVERTED, "个", 2);
    ExpectSegment(2, 1, SS_FOCUSED, "袂", 2);
}

TEST_F(BufferConversionTest, Convert_boe1) {
    input("boe");
    spacebar(1);
    ExpectCandidatesHidden();
    ExpectSegment(1, 0, SS_FOCUSED, "未", 1);
}

TEST_F(BufferConversionTest, Convert_boe2) {
    input("boe");
    spacebar(8);
    ExpectSegment(2, 0, SS_FOCUSED, "無", 3);
    ExpectSegment(2, 1, SS_COMPOSING, " e", 3);
}

TEST_F(BufferConversionTest, Convert_boe3) {
    input("boe");
    spacebar(9);
    ExpectSegment(2, 0, SS_FOCUSED, "bô", 4);
    ExpectSegment(2, 1, SS_COMPOSING, " e", 4);
}

TEST_F(BufferConversionTest, Convert_boe4) {
    input("boe");
    spacebar(10);
    ExpectSegment(1, 0, SS_FOCUSED, "未", 1);
}

TEST_F(BufferConversionTest, Convert_eee_remove_e) {
    input("eee");
    ExpectSegment(1, 0, SS_COMPOSING, "e e e", 5);
    key_bksp(1);
    ExpectSegment(1, 0, SS_COMPOSING, "e e", 3);
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
    input("kamanne");
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
    input("boho");
    spacebar(2);
    enter();
    ExpectSegment(3, 0, SS_CONVERTED, "bô", 4);
    ExpectSegment(3, 1, SS_UNMARKED);
    ExpectSegment(3, 2, SS_FOCUSED);
}

TEST_F(BufferNavigationTest, Focus_moves_to_end) {
    input("hobo");
    spacebar(1);
    curs_left(1);
    input("a");
    spacebar(1);
    ExpectCaret(3);
}

TEST_F(BufferMgrTest, DISABLED_TmpTest) {
    input("iniauchiaheanneoupoetemthangchhikimhiahanahesitihiasisinithia");
    input("n");
}

//+---------------------------------------------------------------------------
//
// Candidate navigation
//
//----------------------------------------------------------------------------

struct CandidateNavigationTest : public BufferMgrTest {};

TEST_F(CandidateNavigationTest, Focus_boe) {
    input("boe");
    curs_down(1);
    ExpectSegment(1, 0, SS_FOCUSED, "未", 1);
    ExpectCandidateSize(10);
}

TEST_F(CandidateNavigationTest, Focus_taichi) {
    input("taichi");
    curs_down(1);
    ExpectSegment(1, 0, SS_FOCUSED, "事志", 2);
    ExpectCandidateSize(3);
}

TEST_F(CandidateNavigationTest, Focus_erase_e) {
    input("e");
    curs_down(1);
    key_bksp(1);
    EXPECT_TRUE(bufmgr->IsEmpty());
}

TEST_F(CandidateNavigationTest, Focus_prev_e) {
    input("ex");
    spacebar(1);
    curs_up(1);
    ExpectSegment(3, 0, SS_FOCUSED, "ē", 3);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 3);
    ExpectSegment(3, 2, SS_CONVERTED, "x", 3);
}

TEST_F(CandidateNavigationTest, Focus_xyz) {
    input("boxyz");
    spacebar(1);
    ExpectSegment(3, 0, SS_FOCUSED, "無", 5);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 5);
    ExpectSegment(3, 2, SS_CONVERTED, "xyz", 5);

    curs_right(1);
    curs_down(1);
    ExpectCandidate("xyz", 0, true);
}

TEST_F(CandidateNavigationTest, Focus_si__bo) {
    input("si--bo");
    curs_down(1);
}

TEST_F(CandidateNavigationTest, Focus_hobo) {
    input("hobo");
    curs_down(1);
    ExpectSegment(2, 0, SS_FOCUSED, "好", 2);
    ExpectSegment(2, 1, SS_CONVERTED, "無", 2);
    curs_down(1);
    ExpectSegment(2, 0, SS_FOCUSED, "好", 4);
    ExpectSegment(2, 1, SS_COMPOSING, " bo", 4);
}

//+---------------------------------------------------------------------------
//
// Candidate selection
//
//----------------------------------------------------------------------------

struct CandidateSelectionTest : public BufferMgrTest {};

TEST_F(CandidateSelectionTest, Select_e) {
    input("e");
    curs_down(2);
    enter();
    ExpectSegment(1, 0, SS_FOCUSED, "兮", 1);
    ExpectCandidatesHidden();
}

TEST_F(CandidateSelectionTest, Select_ebe1) {
    input("ebe");
    auto i = CandidateIndexOf("會");
    spacebar(i);
    enter();
    ExpectSegment(2, 0, SS_CONVERTED, "會", 2);
    ExpectSegment(2, 1, SS_FOCUSED, "未", 2);
}

TEST_F(CandidateSelectionTest, Select_ebe2) {
    input("ebe");
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
    input("ex");
    spacebar(1);
    ExpectSegment(3, 0, SS_FOCUSED, "个", 3);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 3);
    ExpectSegment(3, 2, SS_CONVERTED, "x", 3);
    curs_down(1);
    ExpectSegment(3, 0, SS_FOCUSED, "兮", 3);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 3);
    ExpectSegment(3, 2, SS_CONVERTED, "x", 3);
    enter();
    ExpectSegment(3, 0, SS_CONVERTED, "兮", 3);
    ExpectSegment(3, 1, SS_UNMARKED, " ", 3);
    ExpectSegment(3, 2, SS_FOCUSED, "x", 3);
    curs_right(2);
    curs_down(1);
}

TEST_F(CandidateSelectionTest, Select_goa21) {
    input("goa21");
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
    input(".");
    ExpectBuffer(".", 1);
    ExpectCandidate("。");
}

//+---------------------------------------------------------------------------
//
// Commit buffer
//
//----------------------------------------------------------------------------

struct BufferCommitTest : ::testing::Test, BufferMgrTestBase {
  protected:
    void SetUp() override {
        bufmgr = engine()->buffer_mgr();
        bufmgr->Clear();
    }
    void TearDown() override {
        engine()->database()->ClearNGramsData();
    }
};

TEST_F(BufferCommitTest, NgramsSaved) {
    input("hobo");
    spacebar(1);
    enter();
    EXPECT_EQ(engine()->database()->UnigramCount("好"), 1);
    EXPECT_EQ(engine()->database()->BigramCount({"好", "無"}), 1);
}

} // namespace khiin::engine

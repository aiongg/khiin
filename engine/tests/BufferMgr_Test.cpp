#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BufferMgr.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {
using namespace messages;
using ::testing::Contains;

struct BufferMgrTest : ::testing::Test {
  protected:
    void SetUp() override {
        bufmgr = TestEnv::engine()->buffer_mgr();
        bufmgr->Clear();
    }

    void insert_string(std::string str) {
        for (auto c : str) {
            bufmgr->Insert(c);
        }
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

    void erase_left(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->Erase(CursorDirection::L);
        }
    }

    void erase_right(int n) {
        for (auto i = 0; i < n; i++) {
            bufmgr->Erase(CursorDirection::R);
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
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, Insert_a) {
    bufmgr->Insert('a');
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
    EXPECT_EQ(preedit->cursor_position(), 1);
}

TEST_F(BufferMgrTest, Insert_taichi) {
    bufmgr->Insert('t');
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "t");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('a');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "ta");
    EXPECT_EQ(preedit->cursor_position(), 2);

    bufmgr->Insert('i');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "ta i");
    EXPECT_EQ(preedit->cursor_position(), 4);

    bufmgr->Insert('c');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "tai c");
    EXPECT_EQ(preedit->cursor_position(), 5);

    bufmgr->Insert('h');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "tai ch");
    EXPECT_EQ(preedit->cursor_position(), 6);

    bufmgr->Insert('i');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "tai chi");
    EXPECT_EQ(preedit->cursor_position(), 7);
}

TEST_F(BufferMgrTest, Insert_to7si7) {
    bufmgr->Insert('t');
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "t");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('o');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"to");
    EXPECT_EQ(preedit->cursor_position(), 2);

    bufmgr->Insert('7');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō");
    EXPECT_EQ(preedit->cursor_position(), 2);

    bufmgr->Insert('s');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō s");
    EXPECT_EQ(preedit->cursor_position(), 4);

    bufmgr->Insert('i');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō si");
    EXPECT_EQ(preedit->cursor_position(), 5);

    bufmgr->Insert('7');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"tō sī");
    EXPECT_EQ(preedit->cursor_position(), 5);
}

TEST_F(BufferMgrTest, Candidates_taichi) {
    insert_string("taichi");
    auto cands = get_cand_strings();
    EXPECT_GT(cands.size(), 3);
    EXPECT_THAT(cands, Contains(u8"事志"));
    EXPECT_THAT(cands, Contains(u8"代志"));
    EXPECT_THAT(cands, Contains(u8"tāi-chì"));
}

TEST_F(BufferMgrTest, Reset) {
    insert_string("a");
    bufmgr->Clear();
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Insert_tai7chi) {
    insert_string("tai7chi");
    EXPECT_EQ(display(), u8"tāi chi");
    EXPECT_EQ(caret(), 7);
}

TEST_F(BufferMgrTest, Insert_ian9jin2) {
    insert_string("ian9");
    EXPECT_EQ(display(), u8"iăn");
    insert_string("j");
    EXPECT_EQ(display(), u8"iăn j");
    insert_string("i");
    EXPECT_EQ(display(), u8"iăn ji");
    insert_string("n");
    EXPECT_EQ(display(), u8"iăn jin");
    insert_string("2");
    EXPECT_EQ(display(), u8"iăn jín");
    EXPECT_EQ(caret(), 7);
}

TEST_F(BufferMgrTest, Move_a) {
    insert_string("a");
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 0);
    bufmgr->MoveCaret(CursorDirection::L);
    EXPECT_EQ(caret(), 0);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 1);
    bufmgr->MoveCaret(CursorDirection::R);
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferMgrTest, Move_ah8) {
    insert_string("ah8");
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

TEST_F(BufferMgrTest, Move_sou2i2) {
    insert_string("sou2i2"); // só͘ í
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

TEST_F(BufferMgrTest, MoveType_siongho) {
    insert_string("siongho");
    EXPECT_EQ(display(), "siong ho");
    curs_left(6);
    insert_string("h");
    EXPECT_EQ(display(), "si ho ng ho");
}

TEST_F(BufferMgrTest, Backspace_a) {
    insert_string("a");
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Delete_a) {
    insert_string("a");
    curs_left(1);
    bufmgr->Erase(CursorDirection::R);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Delete_taichi) {
    insert_string("taichi");
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

TEST_F(BufferMgrTest, Delete_taichi_vspace) {
    insert_string("taichi");
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 7);
    curs_left(3);
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 4);
    erase_left(1);
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 3);
    erase_right(1);
    EXPECT_EQ(display(), "tai chi");
    EXPECT_EQ(caret(), 4);
}

TEST_F(BufferMgrTest, Delete_vspace_sibo) {
    insert_string("sibo");
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 5);
    curs_left(2);
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 3);
    erase_left(1);
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 2);
    erase_right(1);
    EXPECT_EQ(display(), "si bo");
    EXPECT_EQ(caret(), 3);
}

TEST_F(BufferMgrTest, RandomLetters) {
    insert_string("bo5wdprsfnlji7");

    EXPECT_EQ(display(), "bô wdprsfnl jī");
    EXPECT_EQ(caret(), 14);

    curs_left(2);
    erase_left(9);

    EXPECT_EQ(display(), "bô jī");
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferMgrTest, Delete_kah8a) {
    insert_string("kah8a");
    EXPECT_EQ(display(), u8"ka\u030dh a");
    EXPECT_EQ(caret(), 6);
    curs_left(3);
    EXPECT_EQ(caret(), 3);
    erase_left(1);
    EXPECT_EQ(display(), "kha");
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferMgrTest, Insert_khin_a) {
    insert_string("--a");
    EXPECT_EQ(display(), u8"\u00b7a");
    EXPECT_EQ(caret(), 2);
}

TEST_F(BufferMgrTest, Insert_khin_ho2_a) {
    insert_string("ho2---a");
    EXPECT_EQ(display(), u8"hó-\u00b7a");
    EXPECT_EQ(caret(), 5);
}

TEST_F(BufferMgrTest, Insert_autokhin) {
    insert_string("a--bobobobo");
    EXPECT_EQ(display(), u8"a ·bo ·bo ·bo ·bo");
    EXPECT_EQ(caret(), 17);
}

TEST_F(BufferMgrTest, Insert_2khins) {
    insert_string("ho2--si7--bo5");
    EXPECT_EQ(display(), u8"hó ·sī ·bô");
    EXPECT_EQ(caret(), 10);
}

TEST_F(BufferMgrTest, Insert_aan2) {
    insert_string("aan2");
    EXPECT_EQ(display(), u8"a án");
}

TEST_F(BufferMgrTest, Insert_len) {
    insert_string("len");
    EXPECT_EQ(display(), u8"len");
}

TEST_F(BufferMgrTest, Insert_poelen) {
    bufmgr->Insert('m');
    EXPECT_EQ(display(), "m");
    EXPECT_EQ(caret(), 1);
    bufmgr->Insert('n');
    EXPECT_EQ(display(), "mn");
    EXPECT_EQ(caret(), 2);
    bufmgr->Insert('g');
    EXPECT_EQ(display(), "mng");
    EXPECT_EQ(caret(), 3);
    bufmgr->Insert('7');
    EXPECT_EQ(display(), u8"mn\u0304g");
    EXPECT_EQ(caret(), 4);
}

TEST_F(BufferMgrTest, Convert_gina) {
    insert_string("gina");
    bufmgr->SelectNextCandidate();
    EXPECT_EQ(display(), u8"囝仔");
}

TEST_F(BufferMgrTest, DISABLED_Convert_sioutoubai) {
    insert_string("si7outoubai");
    bufmgr->SelectNextCandidate();
    auto preedit = get_preedit();
    EXPECT_EQ(display(), u8"是 o\u0358-tó\u0358-bái");
    EXPECT_EQ(preedit->segments().size(), 3);
}

TEST_F(BufferMgrTest, Convert_erase_ho2) {
    insert_string("ho2");
    bufmgr->SelectNextCandidate();
    erase_left(1);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Convert_e) {
    insert_string("e");
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

bool OrEqual(std::string test, std::string v1, std::string v2) {
    return test == v1 || test == v2;
}

TEST_F(BufferMgrTest, Convert_erase_kamanne) {
    insert_string("kamanne");
    bufmgr->SelectNextCandidate();
    ASSERT_PRED3(OrEqual, display(), "咁按呢", "敢按呢");
    erase_left(1);
    ASSERT_PRED3(OrEqual, display(), "咁按", "敢按");
    erase_left(1);
    ASSERT_PRED3(OrEqual, display(), "咁", "敢");
    erase_left(1);
    EXPECT_EQ(display(), u8"");
}

TEST_F(BufferMgrTest, Convert_erase_insert) {
    insert_string("siannebo");
    bufmgr->SelectNextCandidate();
    curs_left(1);
    erase_left(1);
    EXPECT_EQ(display(), u8"是按無");
    EXPECT_EQ(caret(), 2);
    insert_string("h");
    EXPECT_EQ(display(), u8"是按 h 無");
    EXPECT_EQ(caret(), 4);
    insert_string("o");
    EXPECT_EQ(display(), u8"是按 ho 無");
    EXPECT_EQ(caret(), 5);
}

TEST_F(BufferMgrTest, Convert_insert_middle) {
    insert_string("anne");
    bufmgr->SelectNextCandidate();
    curs_left(1);
    insert_string("h");
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 3);
    insert_string("o");
    EXPECT_EQ(display(), u8"按 ho 呢");
    bufmgr->SelectNextCandidate();
    EXPECT_EQ(display(), u8"按好呢");
}

TEST_F(BufferMgrTest, Convert_insert_erase) {
    insert_string("anne");
    bufmgr->SelectNextCandidate();
    curs_left(1);
    insert_string("ho");
    erase_left(2);
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments_size(), 1);
    EXPECT_EQ(display(), u8"按呢");
}

TEST_F(BufferMgrTest, Focus_element) {
    insert_string("kamanne");
    bufmgr->SelectNextCandidate();

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

TEST_F(BufferMgrTest, Convert_e5) {
    insert_string("e5");
    auto cands = get_cand_strings();
    EXPECT_EQ(cands.size(), 4);

    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"ê");
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::COMPOSING);
    EXPECT_EQ(caret(), 1);
    bufmgr->SelectNextCandidate();
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), cands[0]);
    EXPECT_EQ(preedit->segments().at(0).status(), SegmentStatus::FOCUSED);
    EXPECT_EQ(caret(), 1);
}

TEST_F(BufferMgrTest, TmpTest) {
    insert_string("iniauchiaheanneoupoetemthangchhikimhiahanahesitihiasisinithia");
    insert_string("n");
}

} // namespace
} // namespace khiin::engine

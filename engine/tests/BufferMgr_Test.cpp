#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BufferMgr.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {
using namespace messages;

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
            bufmgr->MoveCaret(CursorDirection::L);
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

    CandidateList* get_candidates() {
        auto candlist = CandidateList::default_instance().New();
        bufmgr->GetCandidates(candlist);
        return candlist;
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

TEST_F(BufferMgrTest, Insert_pengan) {
    bufmgr->Insert('p');
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "p");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('e');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "pe");
    EXPECT_EQ(preedit->cursor_position(), 2);

    bufmgr->Insert('n');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "pen");
    EXPECT_EQ(preedit->cursor_position(), 3);

    bufmgr->Insert('g');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng");
    EXPECT_EQ(preedit->cursor_position(), 4);

    bufmgr->Insert('a');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng a");
    EXPECT_EQ(preedit->cursor_position(), 6);

    bufmgr->Insert('n');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng an");
    EXPECT_EQ(preedit->cursor_position(), 7);
}

TEST_F(BufferMgrTest, Insert_a2bo5) {
    bufmgr->Insert('a');
    auto preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('2');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('b');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á b");
    EXPECT_EQ(preedit->cursor_position(), 3);

    bufmgr->Insert('o');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á bo");
    EXPECT_EQ(preedit->cursor_position(), 4);

    bufmgr->Insert('5');
    preedit = get_preedit();
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á bô");
    EXPECT_EQ(preedit->cursor_position(), 4);
}

TEST_F(BufferMgrTest, Candidates_pengan) {
    insert_string("pengan");
    auto clist = get_candidates();
    EXPECT_GT(clist->candidates().size(), 20);
    EXPECT_EQ(clist->candidates().at(0).value(), u8"平安");
}

TEST_F(BufferMgrTest, Reset) {
    insert_string("a");
    bufmgr->Clear();
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Insert_peng5an) {
    insert_string("peng5an");
    EXPECT_EQ(display(), u8"pêng an");
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

TEST_F(BufferMgrTest, Move_hahnn8) {
    insert_string("hahnn8");
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
    EXPECT_EQ(display(), "si hong ho");
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

TEST_F(BufferMgrTest, Delete_pengan) {
    insert_string("pengan");
    EXPECT_EQ(display(), "peng an");
    EXPECT_EQ(caret(), 7);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "peng a");
    EXPECT_EQ(caret(), 6);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "peng");
    EXPECT_EQ(caret(), 4);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "pen");
    EXPECT_EQ(caret(), 3);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "pe");
    EXPECT_EQ(caret(), 2);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "p");
    EXPECT_EQ(caret(), 1);
    bufmgr->Erase(CursorDirection::L);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Delete_vspace_pengan) {
    insert_string("pengan");
    EXPECT_EQ(display(), "peng an");
    EXPECT_EQ(caret(), 7);
    curs_left(2);
    EXPECT_EQ(display(), "peng an");
    EXPECT_EQ(caret(), 5);
    erase_left(1);
    EXPECT_EQ(display(), "peng an");
    EXPECT_EQ(caret(), 4);
    erase_right(1);
    EXPECT_EQ(display(), "peng an");
    EXPECT_EQ(caret(), 5);
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

TEST_F(BufferMgrTest, Delete_siokhoaa) {
    insert_string("siokhoaa");
    EXPECT_EQ(display(), "sio khoa a");
    EXPECT_EQ(caret(), 10);
    erase_left(1);
    EXPECT_EQ(display(), "sio khoa");
    EXPECT_EQ(caret(), 8);
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
    insert_string("a--babababa");
    EXPECT_EQ(display(), u8"a ·ba ·ba ·ba ·ba");
    EXPECT_EQ(caret(), 17);
}

TEST_F(BufferMgrTest, Insert_2khins) {
    insert_string("ho2--a--boe");
    EXPECT_EQ(display(), u8"hó \u00b7a \u00b7boe");
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
    insert_string("poelen");
    EXPECT_EQ(display(), u8"poe len");
}

TEST_F(BufferMgrTest, Convert_gina) {
    insert_string("gina");
    bufmgr->SelectNextCandidate();
    EXPECT_EQ(display(), u8"囝仔");
}

TEST_F(BufferMgrTest, Convert_sioutoubai) {
    insert_string("si7outoubai");
    bufmgr->SelectNextCandidate();
    auto preedit = get_preedit();
    EXPECT_EQ(display(), u8"是 o\u0358-tó\u0358-bái");
    EXPECT_EQ(preedit->segments().size(), 3);
}

TEST_F(BufferMgrTest, Convert_erase) {
    insert_string("ho2");
    bufmgr->SelectNextCandidate();
    erase_left(1);
    EXPECT_EQ(display(), "");
    EXPECT_EQ(caret(), 0);
}

TEST_F(BufferMgrTest, Convert_ho_twice) {
    insert_string("ho");
    bufmgr->SelectNextCandidate();
    bufmgr->SelectNextCandidate();
    auto candlist = get_candidates();
    EXPECT_GT(candlist->candidates_size(), 2);
}

TEST_F(BufferMgrTest, Convert_erase_middle) {
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
}

} // namespace
} // namespace khiin::engine

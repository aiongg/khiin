#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BufferMgr.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {

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

    BufferMgr *bufmgr = nullptr;
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, Insert_a) {
    bufmgr->Insert('a');
    auto preedit = messages::Preedit::default_instance().New();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
    EXPECT_EQ(preedit->cursor_position(), 1);
}

TEST_F(BufferMgrTest, Insert_pengan) {
    auto preedit = messages::Preedit::default_instance().New();

    bufmgr->Insert('p');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "p");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('e');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "pe");
    EXPECT_EQ(preedit->cursor_position(), 2);

    bufmgr->Insert('n');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "pen");
    EXPECT_EQ(preedit->cursor_position(), 3);

    bufmgr->Insert('g');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng");
    EXPECT_EQ(preedit->cursor_position(), 4);

    bufmgr->Insert('a');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng a");
    EXPECT_EQ(preedit->cursor_position(), 6);

    bufmgr->Insert('n');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng an");
    EXPECT_EQ(preedit->cursor_position(), 7);
}

TEST_F(BufferMgrTest, Insert_a2bo5) {
    auto preedit = messages::Preedit::default_instance().New();

    bufmgr->Insert('a');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('2');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á");
    EXPECT_EQ(preedit->cursor_position(), 1);

    bufmgr->Insert('b');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á b");
    EXPECT_EQ(preedit->cursor_position(), 3);

    bufmgr->Insert('o');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á bo");
    EXPECT_EQ(preedit->cursor_position(), 4);

    bufmgr->Insert('5');
    preedit->Clear();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), u8"á bô");
    EXPECT_EQ(preedit->cursor_position(), 4);
}

TEST_F(BufferMgrTest, Candidates_pengan) {
    auto clist = messages::CandidateList::default_instance().New();

    insert_string("pengan");
    bufmgr->GetCandidates(clist);

    EXPECT_EQ(clist->candidates().size(), 1);
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

} // namespace
} // namespace khiin::engine

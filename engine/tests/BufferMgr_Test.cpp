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
    BufferMgr *bufmgr = nullptr;
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, InsertLetter) {
    bufmgr->Insert('a');
    auto preedit = messages::Preedit::default_instance().New();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
    EXPECT_EQ(preedit->cursor_position(), 1);
}

TEST_F(BufferMgrTest, InsertSimpleWord) {
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

TEST_F(BufferMgrTest, InsertTone) {
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

} // namespace
} // namespace khiin::engine

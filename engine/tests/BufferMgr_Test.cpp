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
    bufmgr->Insert('p');
    bufmgr->Insert('e');
    bufmgr->Insert('n');
    bufmgr->Insert('g');
    bufmgr->Insert('a');
    bufmgr->Insert('n');
    auto preedit = messages::Preedit::default_instance().New();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "peng an");
    EXPECT_EQ(preedit->cursor_position(), 7);
}

} // namespace
} // namespace khiin::engine

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "BufferMgr.h"
#include "Engine.h"

namespace khiin::engine {
namespace {

struct BufferMgrTest : ::testing::Test {
  protected:
    void SetUp() override {
        engine = Engine::Create();
        bufmgr = engine->buffer_mgr();
    }
    Engine *engine;
    BufferMgr *bufmgr;
};

TEST_F(BufferMgrTest, Loads) {
    EXPECT_TRUE(bufmgr);
}

TEST_F(BufferMgrTest, DISABLED_InsertLetter) {
    bufmgr->Insert('a');
    auto preedit = messages::Preedit::default_instance().New();
    bufmgr->BuildPreedit(preedit);
    EXPECT_EQ(preedit->segments().size(), 1);
    EXPECT_EQ(preedit->segments().at(0).value(), "a");
}

} // namespace
} // namespace khiin::engine

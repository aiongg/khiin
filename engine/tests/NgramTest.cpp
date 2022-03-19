#include "BufferMgrBaseTest.h"

#include "engine/Database.h"

namespace khiin::engine {

struct NgramTest : ::testing::Test, BufferMgrTestBase {
  protected:
    void SetUp() override {
        bufmgr = TestEnv::engine()->buffer_mgr();
        bufmgr->Clear();
    }

    void TearDown() override {
        TestEnv::engine()->database()->ClearNGramsData();
    }
};

} // namespace khiin::engine
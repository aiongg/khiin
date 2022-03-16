#include "BufferMgrBaseTest.h"

namespace khiin::engine {
using namespace proto;

struct BasicModeTest : ::testing::Test, BufferMgrTestBase {
  protected:
    void SetUp() override {
        bufmgr = TestEnv::engine()->buffer_mgr();
        bufmgr->Clear();

        auto conf = AppConfig();
        conf.set_input_mode(IM_BASIC);
        TestEnv::engine()->config()->UpdateAppConfig(conf);
    }

    void TearDown() override {
        auto conf = AppConfig();
        conf.set_input_mode(IM_CONTINUOUS);
        TestEnv::engine()->config()->UpdateAppConfig(conf);
    }
};

TEST_F(BasicModeTest, Insert_a) {
    input("a");
    ExpectBuffer("a", 1);
}

TEST_F(BasicModeTest, Insert_hobo) {
    input("hobo");
    ExpectBuffer("ho bo", 5);
    ExpectCandidate("好");
    ExpectCandidate("hó");
}

TEST_F(BasicModeTest, Select_anne) {
    input("anne");
    ExpectBuffer("an ne", 5);
    curs_down(4);
    ExpectBuffer("安 ne", 4);
    curs_up(1);
    ExpectBuffer("án-ne", 5);
}

} // namespace khiin::engine

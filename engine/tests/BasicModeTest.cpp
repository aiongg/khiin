#include "BufferMgrTestBase.h"

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

TEST_F(BasicModeTest, Insert_e5) {
    input("e5");
    ExpectCandidateSize(4);
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

TEST_F(BasicModeTest, Delete_a2) {
    input("a2");
    ExpectBuffer("á", 1);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BasicModeTest, Delete_aa) {
    input("a");
    spacebar(1);
    input("a");
    key_bksp(1);
    ExpectSegment(1, 0, SS_FOCUSED);
    key_bksp(1);
    ExpectEmpty();
}

TEST_F(BasicModeTest, Insert_u7h) {
    input("u7h");
    ExpectBuffer("ū h", 3);
}

} // namespace khiin::engine

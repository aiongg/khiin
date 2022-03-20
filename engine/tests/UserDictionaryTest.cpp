#include <gtest/gtest.h>

#include "proto/proto.h"

#include "data/UserDictionary.h"
#include "data/UserDictionaryParser.h"
#include "data/Models.h"

#include "BufferMgrTestBase.h"
#include "TestEnv.h"

namespace khiin::engine {
using namespace proto;

constexpr auto kUserDictionaryFile = "khiin_user.txt";

TEST(UserDictionaryParserTest, ParseFile) {
    auto udp = UserDictionaryParser::LoadFile(kUserDictionaryFile);

    auto has_next = udp->Advance();
    EXPECT_TRUE(has_next);
    auto next = udp->GetRow();
    EXPECT_EQ(next.first, "khiin");
    EXPECT_EQ(next.second, "起引");

    has_next = udp->Advance();
    EXPECT_TRUE(has_next);
    next = udp->GetRow();
    EXPECT_EQ(next.first, "dog");
    EXPECT_EQ(next.second, "káu-á");

    has_next = udp->Advance();
    EXPECT_TRUE(has_next);
    next = udp->GetRow();
    EXPECT_EQ(next.first, "no");
    EXPECT_EQ(next.second, "の");

    has_next = udp->Advance();
    EXPECT_FALSE(has_next);
    next = udp->GetRow();
    EXPECT_TRUE(next.first.empty());
    EXPECT_TRUE(next.second.empty());
}

TEST(UserDictionaryTest, LoadDictionary) {
    auto ud = UserDictionary::Create(kUserDictionaryFile);
    auto result = ud->Search("khiin");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].token->input, "khiin");
    EXPECT_EQ(result[0].token->output, "起引");
    
    result = ud->Search("dog");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].token->input, "dog");
    EXPECT_EQ(result[0].token->output, "káu-á");

    result = ud->Search("no");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].token->input, "no");
    EXPECT_EQ(result[0].token->output, "の");
}

struct UserDictionaryBufferTest : ::testing::Test, BufferMgrTestBase {
    void SetUp() override {
        bufmgr = engine()->buffer_mgr();
        bufmgr->Clear();
        engine()->LoadUserDictionary(kUserDictionaryFile);
    }
    void TearDown() override {
        engine()->LoadUserDictionary("");
        auto conf = AppConfig();
        conf.set_input_mode(IM_CONTINUOUS);
        TestEnv::engine()->config()->UpdateAppConfig(conf);
    }
};

TEST_F(UserDictionaryBufferTest, UserDictionaryCandidates) {
    auto conf = AppConfig();
    conf.set_input_mode(IM_BASIC);
    TestEnv::engine()->config()->UpdateAppConfig(conf);
    
    input("khiin");
    ExpectCandidate("起引");
    
    bufmgr->Clear();
    input("dog");
    ExpectBuffer("dog", 3);
    ExpectCandidate("káu-á");

    bufmgr->Clear();
    input("no");
    ExpectBuffer("no", 2);
    ExpectCandidate("の");

    bufmgr->Clear();
    conf.set_input_mode(IM_CONTINUOUS);
    TestEnv::engine()->config()->UpdateAppConfig(conf);
    
    input("khiin");
    ExpectCandidate("起引");
    
    bufmgr->Clear();
    input("dog");
    ExpectBuffer("dog", 3);
    ExpectCandidate("káu-á");

    bufmgr->Clear();
    input("no");
    ExpectBuffer("no", 2);
    ExpectCandidate("の");
}

} // namespace khiin::engine

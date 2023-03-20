#include <gtest/gtest.h>

#include "proto/proto.h"

#include "data/UserDictionary.h"
#include "data/UserDictionaryParser.h"
#include "data/Models.h"

#include "BufferMgrTestBase.h"
#include "TestEnv.h"

namespace khiin::engine {
using namespace proto;

constexpr auto kUserDictionaryFile = "khiin_userdb.txt";

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
    EXPECT_EQ(result[0].input, "khiin");
    EXPECT_EQ(result[0].output, "起引");
    
    result = ud->Search("dog");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].input, "dog");
    EXPECT_EQ(result[0].output, "káu-á");

    result = ud->Search("no");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].input, "no");
    EXPECT_EQ(result[0].output, "の");
}

TEST(UserDictionaryTest, InvalidFile) {
    auto ud = UserDictionary::Create("random-invalid-filename.txt");
    EXPECT_TRUE(ud != nullptr);
}

struct UserDictionaryBufferTest : ::testing::Test, BufferMgrTestBase {
    void SetUp() override {
        bufmgr = engine()->buffer_mgr();
        bufmgr->Clear();
        engine()->LoadUserDictionary(kUserDictionaryFile);
    }
    void TearDown() override {
        engine()->LoadUserDictionary("");
        SetInputMode(IM_CONTINUOUS);
    }
};

TEST_F(UserDictionaryBufferTest, Basic_khiin) {
    SetInputMode(IM_BASIC);
    input("khiin");
    ExpectCandidate("起引");
}
TEST_F(UserDictionaryBufferTest, Basic_dog) {
    SetInputMode(IM_BASIC);
    input("dog");
    ExpectBuffer("dog", 3);
    ExpectCandidate("káu-á");
}
TEST_F(UserDictionaryBufferTest, Basic_no) {
    SetInputMode(IM_BASIC);
    input("no");
    ExpectBuffer("no", 2);
    ExpectCandidate("の");
}

TEST_F(UserDictionaryBufferTest, Continuous_khiin) {
    SetInputMode(IM_CONTINUOUS);
    input("khiin");
    ExpectCandidate("起引");
}

TEST_F(UserDictionaryBufferTest, Continuous_dog) {
    SetInputMode(IM_CONTINUOUS);
    input("dog");
    ExpectBuffer("dog", 3);
    ExpectCandidate("káu-á");
}

TEST_F(UserDictionaryBufferTest, Continuous_no) {
    SetInputMode(IM_CONTINUOUS);
    input("no");
    ExpectBuffer("no", 2);
    ExpectCandidate("の");
}

} // namespace khiin::engine

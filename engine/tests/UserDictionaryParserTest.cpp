#include <gtest/gtest.h>

#include "data/UserDictionary.h"
#include "data/UserDictionaryParser.h"

namespace khiin::engine {

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

}

}

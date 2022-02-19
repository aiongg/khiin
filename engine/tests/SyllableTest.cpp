#include <gtest/gtest.h>

/*
#include "Syllable.h"

namespace khiin::engine {
namespace {

class SyllableTest : public ::testing::Test {
  protected:
    void SetUp() override {}
};

TEST_F(SyllableTest, TestDefaultConstructor) {
    auto s = Syllable();
    EXPECT_EQ(s.value(), "");
}

TEST_F(SyllableTest, TestInsert) {
    auto s = Syllable();
    auto i = s.InsertAt(0, 'a');
    EXPECT_EQ(s.value(), "a");
    EXPECT_EQ(i, 1);

    i = s.InsertAt(1, '2');
    EXPECT_EQ(s.value(), "á");
    EXPECT_EQ(i, 1);

    s = Syllable();
    i = s.InsertAt(0, 'i');
    EXPECT_EQ(s.value(), "i");
    EXPECT_EQ(i, 1);

    i = s.InsertAt(i, '2');
    EXPECT_EQ(s.value(), "í");
    EXPECT_EQ(i, 1);

    i = s.InsertAt(i, 'a');
    EXPECT_EQ(s.value(), "iá");
    EXPECT_EQ(i, 2);
}

TEST_F(SyllableTest, TestErase) {
    //auto s = Syllable();
    //auto i = s.InsertAt(0, 'a');
    //EXPECT_EQ(s.value(), "a");
    //EXPECT_EQ(i, 1);

    //i = s.InsertAt(1, '2');
    //EXPECT_EQ(s.value(), "á");
    //EXPECT_EQ(i, 1);
}

} // namespace
} // namespace khiin::engine

*/
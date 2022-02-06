#include <gtest/gtest.h>

#include "unicode_utils.h"

namespace khiin::engine::unicode {
namespace {

TEST(UnicodeUtilsTest, starts_with_alnum_test) {
    EXPECT_TRUE(starts_with_alnum("a"));
    EXPECT_TRUE(starts_with_alnum("A"));
    EXPECT_TRUE(starts_with_alnum("á"));
    EXPECT_TRUE(starts_with_alnum("Á"));
    EXPECT_TRUE(starts_with_alnum("1"));
    EXPECT_TRUE(starts_with_alnum("n\u030dgh"));
    EXPECT_FALSE(starts_with_alnum("-a"));
}

TEST(UnicodeUtilsTest, ends_with_alnum_test) {
    EXPECT_TRUE(ends_with_alnum("a"));
    EXPECT_TRUE(ends_with_alnum("A"));
    EXPECT_TRUE(ends_with_alnum("á"));
    EXPECT_TRUE(ends_with_alnum("Á"));
    EXPECT_TRUE(ends_with_alnum("1"));
    EXPECT_TRUE(ends_with_alnum("n\u030d"));
    EXPECT_TRUE(ends_with_alnum("o\u0358"));
    EXPECT_TRUE(ends_with_alnum("\u1e73"));
    EXPECT_TRUE(ends_with_alnum("o\u0324"));
    EXPECT_FALSE(ends_with_alnum("a-"));
}

} // namespace
} // namespace khiin::engine

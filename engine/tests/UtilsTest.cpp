#include <gtest/gtest.h>

#include "engine/utils.h"

namespace khiin::engine::utils {
namespace {

TEST(UtilsTest, FormatString) {
    std::string fmt = "%s bar";
    std::string var = "foo";
    auto ret = format(fmt, var);
    EXPECT_EQ(ret, "foo bar");

    fmt = "foo %s %s %s";
    ret = format(fmt, "bar", "baz", "qux");
    EXPECT_EQ(ret, "foo bar baz qux");
}

} // namespace
} // namespace khiin::engine::utils
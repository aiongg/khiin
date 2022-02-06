#include <gtest/gtest.h>

#include "unicode_utils.h"

namespace khiin::engine::unicode {
namespace {

TEST(UnicodeUtilsTest, glyph_category_alnum) {
    auto cat = GlyphCategory::Alnum;
    EXPECT_EQ(cat, start_glyph_type("a"));
    EXPECT_EQ(cat, start_glyph_type("a"));
    EXPECT_EQ(cat, start_glyph_type("A"));
    EXPECT_EQ(cat, start_glyph_type("á"));
    EXPECT_EQ(cat, start_glyph_type("Á"));
    EXPECT_EQ(cat, start_glyph_type("1"));
    EXPECT_EQ(cat, start_glyph_type("n\u030dgh"));
    EXPECT_EQ(cat, end_glyph_type("a"));
    EXPECT_EQ(cat, end_glyph_type("a"));
    EXPECT_EQ(cat, end_glyph_type("A"));
    EXPECT_EQ(cat, end_glyph_type("á"));
    EXPECT_EQ(cat, end_glyph_type("Á"));
    EXPECT_EQ(cat, end_glyph_type("1"));
    EXPECT_EQ(cat, end_glyph_type("n\u030dgh"));
}

TEST(UnicodeUtilsTest, glyph_category_other) {
    
}

} // namespace
} // namespace khiin::engine

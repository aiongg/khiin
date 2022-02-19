#include <gtest/gtest.h>

#include "unicode_utils.h"

namespace khiin::unicode {
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

TEST(UnicodeUtilsTest, SurrogatePair) {
    auto test = std::string(u8"𫔘");
    EXPECT_EQ(unicode::u8_size(test), 1);
    auto cp = utf8::peek_next(test.begin(), test.end());
    EXPECT_EQ(cp, 0x2b518);
}

} // namespace
} // namespace khiin::engine

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestEnv.h"
#include "Segmenter.h"

namespace khiin::engine {
namespace {

TEST(SegmenterTest, SegmentText) {
    auto segments = Segmenter::SegmentText2(TestEnv::engine(), "taichi");
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments[0].type, SegmentType::Splittable);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 6);
}

} // namespace
} // namespace khiin::engine

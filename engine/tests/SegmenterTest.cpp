#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "input/Segmenter.h"

#include "TestEnv.h"

namespace khiin::engine {
namespace {

struct SegmenterTest : ::testing::Test, TestEnv {};
TEST_F(SegmenterTest, Segment_taichi) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "taichi");
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments[0].type, SegmentType::Splittable);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 6);
}

TEST_F(SegmenterTest, Segment_mn) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "mn");
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments[0].type, SegmentType::WordPrefix);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 2);
}

TEST_F(SegmenterTest, Segment_tehch) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "tehch");
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments[0].type, SegmentType::Splittable);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 3);
    EXPECT_EQ(segments[1].type, SegmentType::WordPrefix);
    EXPECT_EQ(segments[1].start, 3);
    EXPECT_EQ(segments[1].size, 2);
}

TEST_F(SegmenterTest, Segment_chx) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "chx");
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments[0].type, SegmentType::None);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 3);
}

TEST_F(SegmenterTest, Segment_ian9jin2) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "ian9jin2");
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments[0].type, SegmentType::SyllablePrefix);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 4);
    EXPECT_EQ(segments[1].type, SegmentType::SyllablePrefix);
    EXPECT_EQ(segments[1].start, 4);
    EXPECT_EQ(segments[1].size, 4);
}

TEST_F(SegmenterTest, Segment_goabobehkhi) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "goabobehkhi");
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments[0].type, SegmentType::Splittable);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 11);
}

TEST_F(SegmenterTest, Segment_u__bo) {
    auto segments = Segmenter::SegmentText(TestEnv::engine(), "u--bo");
    EXPECT_EQ(segments.size(), 3);
    EXPECT_EQ(segments[0].type, SegmentType::Splittable);
    EXPECT_EQ(segments[0].start, 0);
    EXPECT_EQ(segments[0].size, 1);
    EXPECT_EQ(segments[1].type, SegmentType::Hyphens);
    EXPECT_EQ(segments[1].start, 1);
    EXPECT_EQ(segments[1].size, 2);
    EXPECT_EQ(segments[2].type, SegmentType::Splittable);
    EXPECT_EQ(segments[2].start, 3);
    EXPECT_EQ(segments[2].size, 2);
}

} // namespace
} // namespace khiin::engine

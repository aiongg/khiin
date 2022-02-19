#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestEnv.h"
#include "Segmenter.h"

namespace khiin::engine {
namespace {

TEST(SegmenterTest, SegmentText) {
    auto result = std::vector<BufferElement>();
    Segmenter::SegmentText(TestEnv::engine(), "taichi", result);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].raw(), "taichi");
}

} // namespace
} // namespace khiin::engine

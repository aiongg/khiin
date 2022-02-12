#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestEnv.h"
#include "Segmenter.h"

namespace khiin::engine {
namespace {

TEST(SegmenterTest, SegmentText) {
    auto result = std::vector<BufferElement>();
    Segmenter::SegmentText(TestEnv::engine(), "pengan", result);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].raw(), "pengan");
}

} // namespace
} // namespace khiin::engine

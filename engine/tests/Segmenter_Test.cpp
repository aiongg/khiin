#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestEnv.h"
#include "Segmenter.h"

namespace khiin::engine {
namespace {

TEST(SegmenterTest, SegmentWholeBuffer) {
    auto segmenter = TestEnv::engine()->segmenter();
    auto result = std::vector<BufferElement>();
    auto caret = std::string::npos;
    segmenter->SegmentWholeBuffer("pengan", 6, result, caret);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].raw(), "pengan");
}

} // namespace
} // namespace khiin::engine


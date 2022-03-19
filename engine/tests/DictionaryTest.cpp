#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "engine/Dictionary.h"

#include "TestEnv.h"

namespace khiin::engine {
namespace {

struct DictionaryTest : ::testing::Test, TestEnv {};

TEST_F(DictionaryTest, Loads) {
    auto dict = engine()->dictionary();
}

} // namespace
} // namespace khiin::engine

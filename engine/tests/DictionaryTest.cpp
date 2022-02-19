#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Dictionary.h"
#include "TestEnv.h"

namespace khiin::engine {
namespace {

TEST(DictionaryTest, Loads) {
    auto dict = TestEnv::engine()->dictionary();
}

} // namespace
} // namespace khiin::engine

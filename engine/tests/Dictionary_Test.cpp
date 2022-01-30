#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Dictionary.h"
#include "Engine.h"

namespace khiin::engine {
namespace {

TEST(DictionaryTest, Loads) {
    auto engine = Engine::Create("./");
    auto dict = engine->dictionary();
}

} // namespace
} // namespace khiin::engine

#pragma once

#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "data/Database.h"

#include "Engine.h"
#include "TestEnv.h"

namespace khiin::engine {
static std::unique_ptr<Engine> g_engine = nullptr;

void TestEnv::SetUp() {
    g_engine = Engine::Create("./khiin_test.db");
    g_engine->database()->ClearNGramsData();
}

Engine* TestEnv::engine() {
    return g_engine.get();
}
}

int main(int argc, char *argv[]) {
    auto env = std::make_unique<khiin::engine::TestEnv>();
    ::testing::AddGlobalTestEnvironment(env.get());
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
